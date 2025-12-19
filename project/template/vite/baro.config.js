##> config 
	title = 리엑스 vite 테스트
	basePath = c:/temp/vite		// 설치 기준경로
	templatePath = c:/bpdt/project/template		// 설치 및 소스 반영을 위한 템플릿 경로
	useEmotion = false
	useRouter = true
	projectName = sample01
	useDebug = true
	storePath = src/store


##> config {name=frontend}
	logPath = @[templatePath]/frontend/logs
	template = react-ts
	projectName = frontend
	useTailwind = false

##> config {name=tailwind}
	tailwind-template {
		module.exports = {
		  content: [],
		  theme: {
			extend: {},
		  },
		  plugins: [],
		}	
	}
	tailwind-cmd {
		npm install -D tailwindcss @tailwindcss/postcss autoprefixer
		npx tailwindcss init -p			// Generate your tailwind.config.js and postcss.config.js files. 
		
	}
	tailwind-vars {
		'postcss.config.js' <>
			export default {
			  plugins: {
				tailwindcss: {},
				autoprefixer: {},
			  },
			}
		</>
		content <>["./index.html", "./src/**/*.{js,ts,jsx,tsx}"]</>
		'src/App,css' <>
			@tailwind base;
			@tailwind components;
			@tailwind utilities;
		</>
	}
	cmd-create (
		npm create vite@latest @[projectName] -- --template react
		cd @[projectName]
		@[useStyled] ? npm install styled-components
		@[useRouter] ? npm install react-router-dom
		@[useEmotion] ? npm install @emotion/react @emotion/styled
		@[useTailwind] ? <>
			npm install -D tailwindcss@3 postcss autoprefixer
			npx tailwindcss init -p
			# postcss tailwind.css -o public/build/tailwind.css
		</>
		npm install react-pro-sidebar
		npm install
	)
##> config {name=backend}
	logPath = @[templatePath]/backend/logs


##> app { path='src' globalStorePath=@path(storePath, 'global.js') }
script=null
init=null
layout 
<>
	BrowserRouter
		Navbar
		Sidebar
		SearchOverlay
		Routes 
		<>
			@[ filter(pages,'name|path|element') ],map( page => <route path="@[page.path]" element={@[page.element]} />
		</>
	end --BrowserRouter
</>

template 
<>
	@[ import() ]
	// import { BrowserRouter, Routes, Route } from "react-router-dom";
	import { ToastContainer } from "react-toastify";
	import { useEffect } from "react";
	import { useGlobalStore} from "@[globalStorePath]"
	const App = () => {
		const global = useGlobalStore()
		useEffect( ()=> {@[init]}, []);
		useEffect( ()=> store.movePage(), [store.currentPagePath]);
		@[script]
		@[useDebug] ? console.log('currrentPage =='+store.currentPagePath)
		return ( @[ render(layout) ] )
	}
</>

##> store {name=global}
auto 
<>
	currentPagePath
	loadingPage
	prevPagePath
	apiError
	movePage() {
		if( state.prevPagePath==state.currentPagePath ) return clog('global.routePage 페이지 이동 무시 (이전페이지와 같은페이지 입니다)', state.prevPagePath)
		if( state.loadingPage ) return clog('global.routePage 페이지 이동 무시 (페이지가 로딩중입니다)', state.loadPagePath)
		set({loadingPage:true, prevPagePath:state.currentPagePath})
		setTimeout(()=>set({loadingPage:false}), 150)
	}
</>
template 
<>
	import { create } from 'zustand';
	import { persist } from 'zustand/middleware';
	interface AuthProps { @[interface] }
	export const useAuthStore = create(
	  persist<AuthProps>((set) => ({@[persist]}), {@[state]})
	);
</>

##> page {name=app]
template {	
	import { Routes, Route } from 'react-router-dom'
	@[imports]

	function App() {
		@[stores]
		useEffect(()=>{ @[init] }, [])
		return (
			<Routes> @[routes] </Routes>
		);
	}
	export default App;
}
##> page { path=pages }



##> example {
	pages=@[keyValue(example)].map( cur=>{name:@[cur.name],location:@[cur.savePath]} )
}


mui-stack <>
	import Stack from '@mui/material/Stack';
	import Paper from '@mui/material/Paper';
	import { styled } from '@mui/material/styles';

	// Optional: A styled component for example items
	const Item = styled(Paper)(({ theme }) => ({
	  backgroundColor: theme.palette.mode === 'dark' ? '#1A2027' : '#fff',
	  ...theme.typography.body2,
	  padding: theme.spacing(1),
	  textAlign: 'center',
	  color: theme.palette.text.secondary,
	}));

	function App() {
	  return (
		<div>
		  <h2>Vertical Stack (default)</h2>
		  {/* Default direction is 'column' */}
		  <Stack spacing={2}> 
			<Item>Item 1</Item>
			<Item>Item 2</Item>
			<Item>Item 3</Item>
		  </Stack>

		  <h2>Horizontal Stack</h2>
		  {/* Use the 'direction' prop for horizontal alignment */}
		  <Stack direction="row" spacing={2}>
			<Item>Item 1</Item>
			<Item>Item 2</Item>
			<Item>Item 3</Item>
		  </Stack>
		</div>
	  );
	}

	export default App;
</>

##> type {}
/*
	ReturnType<typeof require> ==> image: require('@/assets/images/dummy/pizza_perfetto.png')
	export const stores: StoreInfo[] = 

	https://dummyjson.com/products/1
*/
	ProductInfo {
		id:num, title, description
		category
		price:num
		discount
	}
	MenuCategory {
		category, subtitle?, dishes: Dishes {
			id:num,name,desc,price:number
			img:ReturnType<typeof require>
			isPopular?bool
		}[]
	}
	StoreMarker {
		id,name
		latitude:num, longitude:num
		deliTime, deliFee: num
		cuisine[]
		rating:num
	}
	StoreInfo {
		id,name
		cuisine[]
		rating:num
		note
		tags[]
		isOPen<>bool, def(false)</>
		deliTime, deliFee: num
	}

##> store { name=menu } 
	menulist = MenuCategory[]
	addMenu() {
		axios(url, param, )
	} 
	