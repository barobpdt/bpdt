
import path from "path";
import webpack from "webpack";


import HtmlWebpackPlugin from "html-webpack-plugin";

const configuration: webpack.Configuration = {
  mode: "",

  
  resolve: {
    
    extensions: [".ts", ".tsx", ".js", ".jsx"],

    
    alias: {
      "@src": path.resolve(__dirname, "../src/"),
    },
  },

  
  entry: "./src/index",

  
  output: {
    path: path.resolve(__dirname, "../dist"),
    filename: "[name].bundle.js",
  },

  
  module: {
    
    rules: [
      
      {
        test: /\.(ts|tsx|js|jsx)$/,
        use: ["babel-loader"],
        exclude: /node_modules/,
      },
      
      {
        test: /\.css$/,
        use: ["style-loader", "css-loader"],
        exclude: /node_modules/,
      },
    ],
  },

  
  plugins: [
    new HtmlWebpackPlugin({
      template: path.join(__dirname, "..", "public", "index.html"),
    }),
  ],
};

export default configuration;
