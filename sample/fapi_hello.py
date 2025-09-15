from fastapi import FastAPI

app = FastAPI()

@app.get("/")
async def read_root():
    return {
        "message": "FastAPI MySQL CRUD Sample",
        "database": f"Connected to {DB_NAME} on {DB_HOST}",
        "docs": "/docs",
        "endpoints": {
            "users": "/users",
            "create_user": "POST /users",
            "get_user": "GET /users/{user_id}",
            "update_user": "PUT /users/{user_id}",
            "delete_user": "DELETE /users/{user_id}"
        }
    }