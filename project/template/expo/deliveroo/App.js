import "./global.css"
import { StatusBar } from 'expo-status-bar';
import { Text, View } from 'react-native';
import { NavigationContainer } from '@react-navigation/native';
import { createNativeStackNavigator } from '@react-navigation/native-stack';
import HomeScreen from "./screens/HomeScreen";
import Order from "./screens/OrderScreen";
import RestaurantScreen from "./screens/RestaurantScreen";
import { store } from './store'
import { Provider } from 'react-redux'
import CartScreen from "./screens/CartScreen";
import Delivery from "./screens/Delivery";
export default function App() {
  const Stack = createNativeStackNavigator();

  return (
    <Provider store={store}>
    <NavigationContainer>
      <Stack.Navigator>
        <Stack.Screen  name="Home" component={HomeScreen} options={{
          headerTitle:() =>(
            <Text  style={{fontSize:15,fontWeight:"bold",textAlign:"center",width:'100%'}}>
              Home
            </Text>
          )
        }} />
        <Stack.Screen  name="Restaurent" component={RestaurantScreen} options={{
          headerTitle:() =>(
            <Text  style={{fontSize:15,fontWeight:"bold",textAlign:"center",width:'100%'}}>
              Restaurant
            </Text>
          )
        }} />
        <Stack.Screen  name="Cart" component={CartScreen} options={{
          presentation:"modal",
          headerShown:false,
        }} />
           <Stack.Screen  name="OrderScreen" component={Order} options={{
            // presentation:"fullScreenModal",
          headerShown:false,
          headerTitle:() =>(
            <Text  style={{fontSize:15,fontWeight:"bold",textAlign:"center",width:'100%'}}>
              Order
            </Text>
          )
        }} />
           <Stack.Screen  name="Delivery" component={Delivery} options={{
            // presentation:"fullScreenModal",
          headerShown:false,
          headerTitle:() =>(
            <Text  style={{fontSize:15,fontWeight:"bold",textAlign:"center",width:'100%'}}>
              Delivery
            </Text>
          )
        }} />
      </Stack.Navigator>
    </NavigationContainer>
    </Provider>
  );
}


