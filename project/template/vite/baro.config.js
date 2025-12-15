##> config 
useMui = true
cmd(
	npm create vite@latest @[projectName] -- --template react
	cd @[projectName]
	npm install
	@[useMui] ? <>
		npm install @mui/material @emotion/react @emotion/styled
	</>
)

example(
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

)

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
	