##> jwt 적용
#ENV.js
import dotenv from "dotenv";
dotenv.config();
export const ENV = {
	JWT_SECRET_KEY:process.env.JWT_SECRET_KEY
	JWT_EXPIRES_IN:process.env.JWT_EXPIRES_IN
	COOKIE_EXPIRES_IN:process.env.COOKIE_EXPIRES_IN
};

#jwtToken.js
import jwt from "jsonwebtoken";
export const sendToken = (user, statusCode, message, res) => {
	const token = jwt.sign({ id: user.id }, ENV.JWT_SECRET_KEY, {
		expiresIn: ENV.JWT_EXPIRES_IN,
	});
	res.status(statusCode).cookie("token", token, {
		expires: new Date(Date.now() + ENV.COOKIE_EXPIRES_IN * 24 * 60 * 60 * 1000),
		httpOnly: true
	}).json({
		success: true,
		user,
		message,
		token,
	});
};

# auth.js
import { sendToken } from "../utils/jwtToken.js";
import crypto from "crypto";
import jwt from "jsonwebtoken";
import ErrorHandler from "./error.js";
import pool from "../config/db.js";

export const catchAsyncErrors = (theFunction) => { return (req, res, next) => Promise.resolve(theFunction(req, res, next)).catch(next) };

export const isAuthenticated = catchAsyncErrors(async (req, res, next) => {
  const { token } = req.cookies;
  if (!token) {
    return next(new ErrorHandler("Please login to access this resource.", 401));
  }
  const decoded = jwt.verify(token, process.env.JWT_SECRET_KEY);

  const user = await database.query(
    "SELECT * FROM users WHERE id = $1 LIMIT 1",
    [decoded.id]
  );
  req.user = user.rows[0];
  next();
});

export const register = catchAsyncErrors(async (req, res, next) => {
  const { name, email, password } = req.body;
  if (!name || !email || !password) {
    return next(new ErrorHandler("Please provide all required fields.", 400));
  }
  if (password.length < 8 || password.length > 16) {
    return next(
      new ErrorHandler("Password must be between 8 and 16 characters.", 400)
    );
  }
  const isAlreadyRegistered = await database.query(
    `SELECT * FROM users WHERE email = $1`,
    [email]
  );
  if (isAlreadyRegistered.rows.length > 0) {
    return next(
      new ErrorHandler("User already registered with this email.", 400)
    );
  }
  const hashedPassword = await bcrypt.hash(password, 10);
  const user = await database.query(
    "INSERT INTO users (name, email, password) VALUES ($1, $2, $3) RETURNING *",
    [name, email, hashedPassword]
  );
  sendToken(user.rows[0], 201, "User registered successfully", res);
});


#error.js
class ErrorHandler extends Error {
  constructor(message, statusCode) {
    super(message);
    this.statusCode = statusCode;
  }
}
export const errorMiddleware = (err, req, res, next) => {
  err.message = err.message || "Internal Server Error";
  err.statusCode = err.statusCode || 500;
  if (err.code === 11000) {
    const message = `Duplicate field value entered`;
    err = new ErrorHandler(message, 400);
  }
  if (err.name === "JsonWebTokenError") {
    const message = "JSON Web Token is invalid, try again";
    err = new ErrorHandler(message, 400);
  }
  if (err.name === "TokenExpiredError") {
    const message = "JSON Web Token has expired, try again";
    err = new ErrorHandler(message, 400);
  }
  if (err.name === "CastError") {
    const message = `Invalid ${err.path}: ${err.value}`;
    err = new ErrorHandler(message, 400);
  }
  const errorMessage = err.errors
    ? Object.values(err.errors)
        .map((error) => error.message)
        .join(" ")
    : err.message;

  return res.status(err.statusCode).json({
    success: false,
    message: errorMessage,
  });
};
export default ErrorHandler;

