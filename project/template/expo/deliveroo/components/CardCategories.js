import { View, Text, ScrollView, TouchableOpacity } from 'react-native'
import React from 'react'
import { Image } from 'react-native'
import { urlFor } from '../sanity'

const CardCategories = ({imgUrl,title}) => {
  return (
    <TouchableOpacity className='relative mr-2'>
        <Image
        source={{
            uri:urlFor(imgUrl).url()
        }}
        className="w-20 h-20 rounded"
        />
      <Text className='absolute bottom-1 left-1 text-white font-bold'>{title}</Text>
    </TouchableOpacity>
  )
}

export default CardCategories