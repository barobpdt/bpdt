import { View, Text } from 'react-native'
import React from 'react'
import { useNavigation } from '@react-navigation/native'
import { useSelector } from 'react-redux';
import { selectCartItem, selectCartTotal } from '../features/CartSlice';
import { TouchableOpacity } from 'react-native';

const CartIcon = () => {
    const navigation = useNavigation();
    const items = useSelector(selectCartItem);
    const CartTotal = useSelector(selectCartTotal);
      const formattedPrice = new Intl.NumberFormat('en-US', {
    style: 'currency',
    currency: 'USD',
  }).format(CartTotal);
  if(items.length ==0) return null;
    return (
        <View className="absolute bottom-10 w-full z-50">
            <TouchableOpacity  onPress={()=>navigation.navigate('Cart')} className="mx-4 p-4 space-x-1 bg-[#00CCBB] flex-row items-center rounded-md">

                <Text className="text-lg bg-white py-1 px-2 mx-1 font-extrabold">{items.length}</Text>
                <Text className="flex-1 text-center text-white text-lg font-extrabold">View Cart</Text>
                <Text className="text-xl  text-white font-extrabold">{formattedPrice}</Text>
            </TouchableOpacity>
        </View>
    )
}

export default CartIcon