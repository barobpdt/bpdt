


import path from "path";
import webpack from "webpack";


import HtmlWebpackPlugin from "html-webpack-plugin";

const configuration: webpack.Configuration = {
  
  resolve: {
    
    extensions: [".ts", ".tsx", ".js", ".jsx"],

    
    alias: {
      "@src": path.resolve(__dirname, "../src/"),
    },
  },

  
  entry: "./src/index",

  
  module: {
    rules: [
      
      {
        test: /\.(ts|tsx|js|jsx)$/,
        use: ["babel-loader"],
        exclude: /node_modules/,
      },
    ],
  },

  
  plugins: [
    new HtmlWebpackPlugin({
      template: path.join(__dirname, "..", "public", "index.html"),
    }),
    
    new webpack.ProvidePlugin({ React: "react" }),
  ],
};

export default configuration;
