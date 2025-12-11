include('classes/common/baro-parse')
include('classes/common/baro-server')
include('classes/common/baro-backend')
include('classes/common/json')
include('classes/common/jobs')
Cf.debug('clear')
@baro.loadPage()
@job.start() 

@baro.initBackend('c:/temp/app_test/backend')

##> dynamic 바인딩
async function dynamicUpdate(id, fieldsToUpdate) {
  let query = 'UPDATE products SET ';
  const params = [];
  const updates = [];
  let paramIndex = 1;

  for (const field in fieldsToUpdate) {
    if (Object.hasOwnProperty.call(fieldsToUpdate, field)) {
      updates.push(`${field} = $${paramIndex}`);
      params.push(fieldsToUpdate[field]);
      paramIndex++;
    }
  }

  query += updates.join(', ');
  query += ` WHERE id = $${paramIndex}`;
  params.push(id);

  // Securely execute the dynamically constructed query
  await pool.query(query, params);
}

async function insertBook(title, author) {
  try {
    // Use placeholders ($1, $2) and pass values in an array
    const result = await sql`
      INSERT INTO books_to_read (title, author) VALUES (${title}, ${author})
      RETURNING id;
    `;
    console.log('Inserted book with ID:', result[0].id);
  } catch (error) {
    console.error('Error inserting book:', error);
  }
}


try {
    // Example: Calling a built-in SQL function (e.g., NOW())
    const { rows: [time] } = await pool.query('SELECT NOW() as current_time;');
    console.log('Current time from database:', time.current_time);

    // Example: Calling a user-defined SQL function
    // Assuming you have a function called 'get_user_count' defined in your Neon database
    const { rows: [count] } = await pool.query('SELECT get_user_count();');
    console.log('Number of users:', count.get_user_count);

    // Example: Executing a query with parameters
    const userId = 123;
    const { rows: [user] } = await pool.query('SELECT * FROM users WHERE id = $1;', [userId]);
    console.log('User data:', user);

} catch (error) {
    console.error('Error executing query:', error);
} finally {
    // Release the connection if using a Client directly, or let the Pool manage it.
    // In serverless functions, the connection might be implicitly managed.
    // For explicit client usage: await client.end();
}

import 'dotenv/config';
import { neon } from '@neondatabase/serverless';
const sql = neon(process.env.DATABASE_URL);
async function setup() {
  try {
    console.log('Connection established');
    // Drop the table if it already exists
    await sql`DROP TABLE IF EXISTS books;`;
    console.log('Finished dropping table (if it existed).');
    // Create a new table
    await sql`
      CREATE TABLE books (
        id SERIAL PRIMARY KEY,
        title VARCHAR(255) NOT NULL,
        author VARCHAR(255),
        publication_year INT,
        in_stock BOOLEAN DEFAULT TRUE
      );
    `;
    console.log('Finished creating table.');
    // Insert a single book record
    await sql`
      INSERT INTO books (title, author, publication_year, in_stock)
      VALUES ('The Catcher in the Rye', 'J.D. Salinger', 1951, true);
    `;
    console.log('Inserted a single book.');
    // Data to be inserted
    const booksToInsert = [
      { title: 'The Hobbit', author: 'J.R.R. Tolkien', publication_year: 1937, in_stock: true },
      { title: '1984', author: 'George Orwell', publication_year: 1949, in_stock: true },
      { title: 'Dune', author: 'Frank Herbert', publication_year: 1965, in_stock: false },
    ];
    // Insert multiple books
    await sql`
      INSERT INTO books (title, author, publication_year, in_stock)
      VALUES (${booksToInsert[0].title}, ${booksToInsert[0].author}, ${booksToInsert[0].publication_year}, ${booksToInsert[0].in_stock}),
             (${booksToInsert[1].title}, ${booksToInsert[1].author}, ${booksToInsert[1].publication_year}, ${booksToInsert[1].in_stock}),
             (${booksToInsert[2].title}, ${booksToInsert[2].author}, ${booksToInsert[2].publication_year}, ${booksToInsert[2].in_stock});
    `;
    console.log('Inserted 3 rows of data.');
  } catch (err) {
    console.error('Connection failed.', err);
  }
}
async function readData() {
  try {
    console.log('Connection established');
    // Fetch all rows from the books table
    const books = await sql`SELECT * FROM books ORDER BY publication_year;`;
    console.log('\n--- Book Library ---');
    books.forEach((book) => {
      console.log(
        `ID: ${book.id}, Title: ${book.title}, Author: ${book.author}, Year: ${book.publication_year}, In Stock: ${book.in_stock}`
      );
    });
    console.log('--------------------\n');
  } catch (err) {
    console.error('Connection failed.', err);
  }
setup();