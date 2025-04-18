


import path from "path";
import webpack from "webpack";
import { merge } from "webpack-merge";
import common from "./webpack.common";


import MiniCssExtractPlugin from "mini-css-extract-plugin";
import CssMinimizerPlugin from "css-minimizer-webpack-plugin";

const configuration: webpack.Configuration = {
  mode: "production",
  devtool: "cheap-module-source-map",
  output: {
    path: path.resolve(__dirname, "../dist"),
    filename: "[name].[contenthash].js",
    clean: true,
  },
  module: {
    rules: [
      {
        test: /\.css$/i,
        use: [MiniCssExtractPlugin.loader, "css-loader"],
      },
    ],
  },
  plugins: [new MiniCssExtractPlugin()],
  optimization: {
    usedExports: true, 
    minimize: true, 
    minimizer: [new CssMinimizerPlugin()], 
  },
};

export default merge(common, configuration);
