const path = require("path")
const HtmlWebpackPlugin = require('html-webpack-plugin')
const MiniCssExtractPlugin = require('mini-css-extract-plugin')
const { CleanWebpackPlugin } = require("clean-webpack-plugin")

module.exports = {
  entry: "./src/index.ts", 
  output: {
    path: path.join(__dirname, "/dist"), 
    filename: "main.js",
  },
  module: {
    rules: [
		 {
        test: /[\.js]$/, 
        exclude: /node_module/,
        use: {
          loader: "babel-loader",
        }
      },
      {
        test: /\.ts$/, 
        exclude: /node_module/,
        use: {
          loader: "ts-loader",
        }
      },
	  {
        test: /\.css$/,
        exclude: "/node_modules",
        use: [
			process.env.NODE_ENV === 'production' ? MiniCssExtractPlugin.loader : 'style-loader'
			, 'css-loader'
		]
      },
      {
        test: /\.scss$/,
        exclude: "/node_modules",
        use: [MiniCssExtractPlugin.loader, "css-loader", "sass-loader"],
      },
    ],
  },
  resolve: {
	modules: [path.join(__dirname, "src"), "node_modules"]
	, extensions: [".ts", ".js"]
  },
  plugins: [
    new HtmlWebpackPlugin({
		template: "./public/index.html"
		, filename: "index.html"
		, templateParameters: {
			env: process.env.NODE_ENV === "development" ? "(개발용)" : "(배포용)"
		}
		, minify: process.env.NODE_ENV === "production" ? { collapseWhitespace: true, removeComments: true,} : false
    })
	, new CleanWebpackPlugin()
  ],
};	
