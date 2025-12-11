import { View, Text, Image, TextInput, ScrollView } from 'react-native'
import React, { useEffect, useLayoutEffect,useState } from 'react'
import { useNavigation } from '@react-navigation/native'
import { SafeAreaView } from 'react-native-safe-area-context';
import { AdjustmentsHorizontalIcon, AdjustmentsVerticalIcon, ChevronDownIcon, MagnifyingGlassIcon, UserIcon } from "react-native-heroicons/solid";
import Categories from '../components/categories';
import FeatureRows from '../components/FeatureRows';
import sanityClient from '../sanity.js'

const HomeScreen = () => {
  const navigation = useNavigation();
  const [features,setFeatures] = useState();
  useLayoutEffect(() => {
    navigation.setOptions({
      headerShown: false
    })
  }, []);

  useEffect(()=>{
    sanityClient.fetch(`*[_type == 'feature'] {
  ...,
  restaurants[]-> {
    ...,
    dishes[]
  }
}`).then((data)=>{
setFeatures(data)
})
  },[])
  return (
    <SafeAreaView >
      <Text>
        {/* Header */}
        <View className='flex-row pb-2 items-center mx-4 space-x-2 px-4'>
          <Image source={{
            uri: 'https://i.ibb.co/99HYr9dr/deliveroo-new-visual-branding-logo-dezeen-2364-ss-1-852x608.jpg'
          }}
            className='h-16 w-16 p-4 rounded-full bg-gray-200'
          />

          <View className='flex-1'>
            <Text className='font-bold text-gray-400 text-xs'>Delivery Now!</Text>
            <Text className='font-bold text-xl'>
              Current Location
              <ChevronDownIcon size={16} color="#000" />
            </Text>
          </View>
          <UserIcon size={24} color="#16C47F" />
        </View>

        {/* search */}
        <View className='flex-row items-center mx-2 px-4'>
          <View className='flex-row items-center flex-1 space-x-2 mx-1 bg-gray-300 px-2'>
            <MagnifyingGlassIcon color="#16C47F" size={20} />
            <TextInput placeholder='Restaurants And cuisines' keyboardType='default' />
          </View>
          <AdjustmentsVerticalIcon color="#16C47F" />
        </View>
      </Text>


      {/* Body */}
      <ScrollView
        className='bg-gray-200'
        contentContainerStyle={{
          paddingBottom: 10
        }}
      >
        {/* Categories */}
        <Categories />
      
        {/* Feature Rows */}
         {features?.map((item)=>(

        <FeatureRows
          key={item._id}
          id={item._id}
          title={item.name}
          description={item.description}
        />
        ))} 
      
      </ScrollView>
    </SafeAreaView>
  )
}

export default HomeScreen