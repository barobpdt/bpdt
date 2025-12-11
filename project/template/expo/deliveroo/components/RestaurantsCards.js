import { View, Text, Image } from 'react-native'
import React from 'react'
import { TouchableOpacity } from 'react-native'
import { MapIcon, MapPinIcon, StarIcon } from 'react-native-heroicons/solid'
import { urlFor } from '../sanity'
import { useNavigation } from '@react-navigation/native'

const RestaurantsCards = ({
    id,
    img,
    title,
    description,
    genre,
    long,
    lat,
    dishes,
    address,
    rating
}) => {
  const navigation = useNavigation();
  return (
    <TouchableOpacity 
    onPress={()=>{
navigation.navigate('Restaurent',{
    id,
    img,
    title,
    description,
    genre,
    long,
    lat,
    dishes,
    address,
    rating
  })
    }}
    className='bg-white mr-3 shadow-slate-200 pb-3 w-64'>
    <Image source={{
                uri: urlFor(img).url()
              }}
           className='h-36 w-64 rounded-sm'
      />
      <View className='pb-4 px-4'>
        <Text className='font-bold pt-2 text-lg'>{title}</Text>
      </View>
      <View className='flex-row items-center space-x-1 px-2'>
        <StarIcon color='#B6CBBD' opacity={0.5} size={22}/>
        <Text>
          <Text className='text-yellow-500'>{rating}</Text> - {genre}
        </Text>
      </View>
      <View className='flex-row items-center space-x-1 pt-2 px-2'>
        <MapPinIcon color='gray' opacity={0.5} size={16}/>
        <Text>
          <Text className='text-sm text-gray-400'>Nearby: {address}</Text> 
        </Text>
      </View>
    
    </TouchableOpacity>
  )
}

export default RestaurantsCards