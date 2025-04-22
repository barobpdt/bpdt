const path = require("path");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const { CleanWebpackPlugin } = require("clean-webpack-plugin");
const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const { WebpackManifestPlugin } = require("webpack-manifest-plugin");
const webpack = require("webpack");
 
module.exports = {
  mode: "development",
  entry: {
    app: path.resolve(__dirname, "..", "./index.js"),
  },
  output: {
    path: path.resolve(__dirname, "dist"),
    filename: "[name].[chunkhash].js",
    chunkFilename: "[name].[chunkhash].js",
  },
  devServer: {
    static: {
      directory: path.join(__dirname, "dist"),
    },
    port: 9000,
    open: true,
    compress: true,
  },
  devtool: "inline-source-map",
  module: {
    rules: [
      {
        test: /\.html$/,
        exclude: "/node_modules",
        use: ["html-loader"],
      },
      {
        test: /\.(js|jsx)$/,
        exclude: "/node_modules",
        use: {
          loader: "babel-loader",
        },
      },
      {
        test: /\.css$/,
        exclude: "/node_modules",
        use: [MiniCssExtractPlugin.loader, "css-loader"],
      },
      {
        test: /\.scss$/,
        exclude: "/node_modules",
        use: [MiniCssExtractPlugin.loader, "css-loader", "sass-loader"],
      },
      {
        
        test: /\.(jpg|jpeg|gif|png|svg|ico)?$/,
        use: [
          {
            loader: "url-loader",
            options: {
              limit: 10000,
              fallback: "file-loader",
              name: "images/[name].[ext]",
            },
          },
        ],
      },
      {
        
        test: /\.(woff|woff2|eot|ttf|otf)$/,
        use: [
          {
            loader: "url-loader",
            options: {
              limit: 10000,
              fallback: "file-loader",
              name: "fonts/[name].[ext]",
            },
          },
        ],
      },
    ],
  },
  plugins: [
    new HtmlWebpackPlugin({
      template: path.resolve(__dirname, "..", "./public/index.html"),
      filename: "index.html",
    }),
    new MiniCssExtractPlugin({
      filename: "static/css/[name].css",
    }),
    new WebpackManifestPlugin({
      fileName: "assets.json",
    }),
    new CleanWebpackPlugin(),
  ],
};	
