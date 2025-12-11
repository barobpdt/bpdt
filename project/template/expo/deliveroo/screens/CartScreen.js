import { View, Text,TouchableOpacity ,Image,ScrollView} from 'react-native'
import React ,{useMemo,useState} from 'react'
import { useNavigation } from '@react-navigation/native'
import { useDispatch, useSelector } from 'react-redux';
import { selectRestuarant } from '../features/restaurant';
import { selectCartItem,removeToCart,selectCartTotal } from '../features/CartSlice';
import { SafeAreaView } from 'react-native-safe-area-context';
import { XCircleIcon } from 'react-native-heroicons/solid'
import { urlFor } from '../sanity';
const CartScreen = () => {
    const navigation = useNavigation();
    const restaurants = useSelector(selectRestuarant);
    const items = useSelector(selectCartItem);
    const dispatch = useDispatch();
    const[groupItemCart,setGroupItemCart] = useState([]);
    useMemo(()=>{
        const groupItem = items.reduce((results,item)=>{
        (results[item.id] = results[item.id] || []).push(item);
        return results;
        },{})
        setGroupItemCart(groupItem);
    },[items])
     const CartTotal = useSelector(selectCartTotal);
      const formattedPrice = new Intl.NumberFormat('en-US', {
    style: 'currency',
    currency: 'USD',
  }).format(CartTotal);
      const formattedPriceTotla = new Intl.NumberFormat('en-US', {
    style: 'currency',
    currency: 'USD',
  }).format(CartTotal + 2.99);
    return (
        <SafeAreaView className="flex-1 bg-white">
            <View className="flex-1 bg-gray-100">
                <View className="p-4 border-b brder-[#16C47F] bg-white shadow-xs">
                    <View>
                        <Text className="text-lg text-center font-bold">Cart</Text>
                        <Text className="text-center text-gray-300">{restaurants.title}</Text>
                    </View>
                    <TouchableOpacity
                    onPress={navigation.goBack}
                    className="rounded-full bg-gray-100 absolute top-2 right-5"
                    >
                        <XCircleIcon color="#16C47F" height={40} width={40}/>
                    </TouchableOpacity>
                </View>
                <View className="flex-row space-x-4 items-center bg-white px-4 py-3 my-5">
                     <Image source={{
                        uri: 'https://i.ibb.co/99HYr9dr/deliveroo-new-visual-branding-logo-dezeen-2364-ss-1-852x608.jpg'
                    }}
                        className='h-10 w-10 p-4 rounded-full bg-gray-200'
                    />
                    <Text className="flex-1 px-3">Deliver in 10 min</Text>
                    <TouchableOpacity>
                    <Text className="text-[#16C47F]">Change</Text>
                    </TouchableOpacity>
                </View>
                <ScrollView className="divide-y divide-gray-400">
                    {Object.entries(groupItemCart).map(([key,items])=>(
                        <View key={key} className="flex-row items-center space-x-3 bg-white py-3 px-5">
                            <Text className="text-[#16C47F] px-1">{items.length}</Text>
                            <Image
                         source={{
                        uri: urlFor(items[0]?.image).url()
                    }}
                    className='h-12 w-12 rounded-full px-3'
                    />
                    <Text className="flex-1 px-1">{items[0]?.name}</Text>
                    <Text className="flex-1 text-gray-500 px-5">{items[0]?.price}$</Text>
                        <TouchableOpacity>
                            <Text
                            onPress={()=>dispatch(removeToCart({id:key}))}
                            className="text-xs text-[#16C47F]"
                            >Remove</Text>
                        </TouchableOpacity>
                        </View>
                    ))}
                </ScrollView>

                <View className="p-5 bg-white mt-5 space-y-4">
                    <View className="flex-row justify-between mb-2">
                        <Text className="text-gray-400">FirstTotla</Text>
                        <Text className="text-gray-400">{formattedPrice}</Text>
                    </View>
                    <View className="flex-row justify-between mb-2">
                        <Text className="text-gray-400">Delivery Fee</Text>
                        <Text className="text-gray-400">${2.99}</Text>
                    </View>
                    <View className="flex-row justify-between mb-2">
                        <Text className="text-gray-400">Order Totla</Text>
                        <Text className="text-gray-400 font-extrabold">{formattedPriceTotla}</Text>
                    </View>
                    <TouchableOpacity className="rounded-lg bg-[#16C47F] p-4" onPress={()=>(navigation.navigate('OrderScreen'))}>
                        <Text className="text-white text-center text-lg">Order</Text>
                    </TouchableOpacity>
                </View>
            </View>
        </SafeAreaView>
    )
}

export default CartScreen