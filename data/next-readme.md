
# Getting Started with Next.js

Next.js is a React framework for building full-stack web applications. It offers features like server-side rendering, static site generation, and API routes, making it a powerful tool for modern web development. This document will guide you through the basics of using Next.js.

## Installation

To start a new Next.js project, you can use the `create-next-app` command:
```
bash
npx create-next-app@latest my-next-app
cd my-next-app
```
This command will:

1.  **Download `create-next-app`:** This is a tool that helps you set up a new Next.js project.
2.  **Create a new folder:** It will create a folder named `my-next-app` (or whatever name you choose) in your current directory.
3.  **Install dependencies:** It will download and install all the necessary packages for your new Next.js project.
4.  **Set up the project:** It will set up a basic project structure for you, including files and folders.

You will then be prompted to choose if you want to use Typescript, ESLint and other options. Once the install is complete, you can start the development server:
```
bash
npm run dev
```
This will start a development server, typically at `http://localhost:3000`.

## Creating Pages

In Next.js, each file in the `pages` directory becomes a route.

-   **`pages/index.js`:** This file corresponds to the root route (`/`).
-   **`pages/about.js`:** This file corresponds to the `/about` route.
-   **`pages/blog/first-post.js`:** This corresponds to `/blog/first-post`.

Here's an example of a simple `pages/about.js` file:
```
javascript
function AboutPage() {
  return <h1>About Us</h1>;
}

export default AboutPage;
```
## Data Fetching

Next.js offers several ways to fetch data:

### `getStaticProps`

Used for static site generation (SSG). This method fetches data at build time.
```
javascript
export async function getStaticProps() {
  const res = await fetch('https://api.example.com/posts');
  const posts = await res.json();

  return {
    props: {
      posts,
    },
  };
}

function Blog({ posts }) {
  return (
    <ul>
      {posts.map((post) => (
        <li key={post.id}>{post.title}</li>
      ))}
    </ul>
  );
}

export default Blog;
```
### `getServerSideProps`

Used for server-side rendering (SSR). This method fetches data on each request.
```
javascript
export async function getServerSideProps(context) {
  const res = await fetch(`https://api.example.com/posts`);
  const posts = await res.json();

  return {
    props: {
      posts,
    },
  };
}

function Blog({ posts }) {
  return (
    <ul>
      {posts.map((post) => (
        <li key={post.id}>{post.title}</li>
      ))}
    </ul>
  );
}

export default Blog;
```
### `getStaticPaths`

Used with `getStaticProps` to specify dynamic routes.
```
javascript
export async function getStaticPaths() {
  const res = await fetch('https://api.example.com/posts');
  const posts = await res.json();

  const paths = posts.map((post) => ({
    params: { id: post.id.toString() },
  }));

  return { paths, fallback: false };
}

export async function getStaticProps({ params }) {
  const res = await fetch(`https://api.example.com/posts/${params.id}`);
  const post = await res.json();

  return {
    props: { post },
  };
}
```
### Client Side fetching
You can fetch data on the client using hooks like `useEffect` and `useState` and the fetch API.

## Routing

Next.js uses file-system-based routing. Each file in the `pages` directory becomes a route.

### Basic Routing

As mentioned earlier, creating a file `pages/about.js` automatically creates an `/about` route.

### Dynamic Routing

You can create dynamic routes using square brackets in the file name. For example:

-   **`pages/posts/[id].js`:** This will match routes like `/posts/1`, `/posts/2`, etc.
-   **`pages/blog/[slug].js`**: This will match routes like `/blog/hello-world`, `/blog/next-js-tutorial`.

### Linking Between Pages

Use the `<Link>` component from `next/link` to navigate between pages:
```
javascript
import Link from 'next/link';

function HomePage() {
  return (
    <div>
      <h1>Welcome to the Home Page</h1>
      <Link href="/about">
        <a>About Us</a>
      </Link>
    </div>
  );
}

export default HomePage;
```
## Building and Deploying

### Building the Application

To build your application for production, run:
```
bash
npm run build
```
This command will create an optimized production build in the `.next` directory.

### Starting the Production Server

To start the production server, run:
```
bash
npm run start
```
This will start a server that serves the built application.

### Deploying

You can deploy a Next.js application to various platforms like:

-   **Vercel:** Ideal for Next.js applications, offers easy deployment and integration.
-   **Netlify:** Another great option for deploying static and server-rendered sites.
-   **AWS, Heroku, DigitalOcean:** For more traditional server environments.

Deployment typically involves connecting your Git repository to the hosting platform and configuring the build command (`npm run build`) and output directory (`.next`).

## Conclusion

This document has covered the essentials of getting started with Next.js. With these foundations, you can begin building powerful and efficient web applications. Remember to consult the official Next.js documentation for more in-depth information.