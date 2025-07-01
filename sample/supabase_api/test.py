from dataModel import User
from supabaseDb import TableSchemaGenerator 
table_name = User.__name__.lower()

schema = TableSchemaGenerator.generate_table_schema(User,'user')
sql = TableSchemaGenerator.generate_sql_ddl(schema)

for field_name, field_info in User.__annotations__.items():
	field = User.__fields__[field_name]
	print(f"{table_name} >> {field_name}", field )


print("@@ test end")	