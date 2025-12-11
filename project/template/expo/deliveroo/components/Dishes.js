import { View, Text, TouchableOpacity } from 'react-native'
import React, { useState } from 'react'
import { Image } from 'react-native';
import { urlFor } from '../sanity';
import { MinusCircleIcon, PlusCircleIcon } from 'react-native-heroicons/solid';
import { useDispatch, useSelector } from 'react-redux';
import { addtoCart, removeToCart, selectCartItem, selectCartItemId } from '../features/CartSlice';

const Dishes = ({
  id, name, price, image, description
}) => {
  const formattedPrice = new Intl.NumberFormat('en-US', {
    style: 'currency',
    currency: 'USD',
  }).format(price);
  const [IsPressed, setIsPressed] = useState(false);
  const dispatch = useDispatch();
  const items = useSelector((state) => selectCartItemId(state, id));
  const addToCart = () => {
    dispatch(addtoCart({ id, name, price, image, description }))
  }
  const RemoveToCart = () => {
    if (!items.length > 0) return;
    dispatch(removeToCart({ id }))
  }
  return (
    <>
      <TouchableOpacity onPress={() => setIsPressed(!IsPressed)} className={`bg-white p-4 border border-gray-200 ${IsPressed && 'border-b-0'}`}>
        <View className="flex-row">
          <View className="flex-1">
            <Text className="text-xl mb-1">{name}</Text>
            <Text className="text-gray-500">{description}</Text>
            <View>
              <Text className="text-gray-400 mt-4">
                {formattedPrice}
              </Text>
            </View>
          </View>
          <View>
            <Image
              style={{
                borderWidth: 1,
                borderColor: "#f3f3f4"
              }}
              source={{
                uri: urlFor(image).url()
              }}
              className="w-20 h-20  bg-gray-300 p-1"
            />
          </View>
        </View>

      </TouchableOpacity>

      {IsPressed && (
        <View className="bg-white px-4">
          <View className="flex-row items-center space-x-2">
            <TouchableOpacity onPress={RemoveToCart}>
              <MinusCircleIcon size={40} color={items.length > 0 ?"#00CCBB":"gray"} />
            </TouchableOpacity>
            <Text>{items.length}</Text>
            <TouchableOpacity onPress={addToCart}>
              <PlusCircleIcon size={40}  color="#00CCBB"/>
            </TouchableOpacity>
          </View>
        </View>
      )}
    </>
  )
}

export default Dishes