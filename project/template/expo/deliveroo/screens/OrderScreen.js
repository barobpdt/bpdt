import { View, Text  } from 'react-native'
import React ,{useEffect} from 'react'
import { SafeAreaView } from 'react-native-safe-area-context';
import * as Animatable from 'react-native-animatable';
import * as Progress from 'react-native-progress';
import { useNavigation } from '@react-navigation/native'

const Order = () => {
    const navigation = useNavigation();
    useEffect(()=>{
        setTimeout(() => {
            navigation.navigate('Delivery')
        }, 4000);
    },[])
  return (
    <SafeAreaView className="bg-[#16C47F] flex-1 justify-center items-center">
        <Animatable.Image  source={require('../assets/deliveroo-delivery.gif')} animation="slideInUp" iterationCount={1} className="w-50 h-50  mr-10" />

      <Animatable.Text animation="slideInUp" iterationCount={1} className="font-bold text-white text-center mt-3">Wating for A Restaurant to accept order</Animatable.Text> 
    <Progress.CircleSnail color={['white', 'green', 'white']} />


    </SafeAreaView>
  )
}

export default Order