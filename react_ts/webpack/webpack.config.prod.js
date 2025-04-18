

const path = require("path");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const { CleanWebpackPlugin } = require("clean-webpack-plugin");
const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const { WebpackManifestPlugin } = require("webpack-manifest-plugin");
const webpack = require("webpack");
const childProcess = require("child_process");
const removeNewLine = (buffer) => {
  return buffer.toString().replace("\n", "");
};
 
module.exports = {
  mode: "production",
  module: "esnext",
  entry: {
    app: path.resolve(__dirname, "..", "./index.js"),
  },
  optimization: {
    splitChunks: {
      cacheGroups: {
        vendor: {
          chunks: "initial",
          name: "vendor",
          enforce: true,
        },
      },
    },
  },
  output: {
    path: path.resolve(__dirname, "..", "./dist"),
    filename: "[name].js",
    chunkFilename: "[name].js",
    publicPath: "/dist/",
  },
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
    new webpack.BannerPlugin({
      banner: `
          Build Date :: 
          Commit Version :: 
          Auth.name :: 
          Auth.email :: 
    `,
    }),
    new webpack.DefinePlugin({
      TWO: JSON.stringify("1+1"),
    }),
    new HtmlWebpackPlugin({
      template: "./public/index.html",
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
