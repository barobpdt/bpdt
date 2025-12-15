@[useProductPage] ? <>
import Navbar from "./components/Navbar";
import HomePage from "./pages/HomePage";
import ProductPage from "./pages/ProductPage";
</>
import { Routes, Route } from "react-router-dom";
import { useThemeStore } from "./store/useThemeStore";
import { Toaster } from "react-hot-toast";

@[useStyled] ? <>
import styled from "styled-components";
const Main = styled.div`
  height:100vh;display:flex;flex-direction:column; align-items:center; justify-contant:center;
`;
</>

function App() {
	const { theme } = useThemeStore();
	return (
	@[useStyled] ? <>
		<Main data-theme={theme}>
			<div>hello world !!!<div>
		</Main>
	</> else <>
		<div className="min-h-screen bg-base-200 transition-colors duration-300" data-theme={theme}>
			<Navbar />
			<Routes>
				<Route path="/" element={<HomePage />} />
				<Route path="/product/:id" element={<ProductPage />} />
			</Routes>
			<Toaster />
		</div>
	</>
	);
}

export default App;
