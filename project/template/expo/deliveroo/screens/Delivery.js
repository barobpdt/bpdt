import { View, Text, TouchableOpacity, Image } from 'react-native'
import React from 'react'
import { SafeAreaView } from 'react-native-safe-area-context';
import { XCircleIcon } from 'react-native-heroicons/solid'
import { useNavigation } from '@react-navigation/native'
import { useDispatch, useSelector } from 'react-redux';
import { selectRestuarant } from '../features/restaurant';
import * as Progress from 'react-native-progress';
import MapView, { Marker } from 'react-native-maps';

const Delivery = () => {
  const navigation = useNavigation();
  const restaurants = useSelector(selectRestuarant);
  return (
    <View className="bg-[#00CCBB] flex-1">
      <SafeAreaView className="z-50">
        <View className="flex-row justify-between items-center p-5">
          <TouchableOpacity onPress={() => navigation.navigate('Home')}>
            <XCircleIcon color="white" size={30} />
          </TouchableOpacity>
          <Text className="font-light text-lg text-white">Delivery</Text>

        </View>
        <View>

          <View className="bg-white shadow-xs mx-3 my-2 z-50  rounded-lg py-6 px-3">
            <View className="flex-row justify-between items-center">
              <View>
                <Text className="font-bold text-lg text-gray-300">Estimated Arrival</Text>
                <Text className="font-bold text-4xl">25 -30 Minutes</Text>
              </View>
              <Image
                source={{
                  uri: 'https://i.ibb.co/99HYr9dr/deliveroo-new-visual-branding-logo-dezeen-2364-ss-1-852x608.jpg'

                }}
                className='h-12 w-12 rounded-full px-3'
              />
            </View>
            <Progress.Bar progress={0.3} width={200} color="#00CCBB" indeterminate={true} className="mt-2" />
            <Text className="font-bold text-sm text-gray-300 mt-3">Your order a {restaurants.title} is being prepred</Text>

          </View>
        </View>
      </SafeAreaView>
                
       <MapView
        style={{ flex: 1, zIndex: 0 }}
        initialRegion={{
          latitude: restaurants.lat,
          longitude:restaurants.long,
          latitudeDelta: 0.005,
          longitudeDelta: 0.005,
        }}
      >
        <Marker
          coordinate={{ latitude: restaurants.lat, longitude: restaurants.long }}
        />
      </MapView> 



    </View>
  )
}

export default Delivery