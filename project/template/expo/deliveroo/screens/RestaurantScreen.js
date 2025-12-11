import { View, Text, ScrollView, Image, TouchableOpacity } from 'react-native'
import React, { useEffect, useLayoutEffect } from 'react'
import { useNavigation, useRoute } from '@react-navigation/native'
import { urlFor } from '../sanity';
import { ArrowLeftIcon, ChevronLeftIcon, ChevronRightIcon, MapPinIcon, QuestionMarkCircleIcon, StarIcon } from 'react-native-heroicons/solid';
import Dishes from '../components/Dishes';
import CartIcon from '../components/CartIcon';
import { useDispatch } from 'react-redux';
import { setRestaurant } from '../features/restaurant';

const RestaurantScreen = () => {
    const navigation = useNavigation();
    const dispatch = useDispatch();
    const { params: {
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
    } } = useRoute();

    useLayoutEffect(() => {
        navigation.setOptions({
            headerShown: false
        })
    }, []);
    useEffect(() => {
        dispatch(setRestaurant({
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
        }));
    }, [])

    return (
        <>
            <CartIcon />
            <ScrollView>
                <View>
                    <Image source={{
                        uri: urlFor(img).url()
                    }}
                        className='h-56 w-full bg-gray-300'
                    />
                    <TouchableOpacity onPress={navigation.goBack} className="absolute top-14 left-5 p-2 bg-gray-300 rounded-full">
                        <ArrowLeftIcon size={20} color="#000" />
                    </TouchableOpacity>
                </View>
                <View className="bg-white">
                    <View className="px-4 pt-4">
                        <Text className="text-3xl font-bold">{title}</Text>

                        <View className="flex-row space-x-2 my-1 ">
                            <View className="flex-row space-x-1 items-center">
                                <StarIcon size={20} color='green' opacity={0.4} />
                                <Text className="text-xs">
                                    <Text className="text-gray-500">{rating}</Text> - {genre}
                                </Text>
                            </View>
                            <View className="flex-row space-x-1 items-center px-3">
                                <MapPinIcon size={20} color='green' opacity={0.4} />
                                <Text className="text-xs">
                                    <Text className="text-gray-500">{address}</Text>
                                </Text>
                            </View>
                        </View>

                        <Text className="text-gray-400 mt-4 pb-4" >{description}</Text>
                    </View>

                    <TouchableOpacity className="flex-row p-4 border-y border-gray-300 items-center space-x-2 ">
                        <QuestionMarkCircleIcon color="gray" size={20} opacity={0.5} />
                        <Text className="pl-2 font-bold flex-1">
                            Have a food allergy ?
                        </Text>
                        <ChevronRightIcon color="green" opacity={0.5} />
                    </TouchableOpacity>
                </View>
                <View className="pb-36">
                    <Text className="px-4 pt-6 mb-3 text-xl font-bold">Menu</Text>
                    {dishes.map((item) => (
                        <Dishes
                            key={item._id}
                            id={item._id}
                            name={item.name}
                            image={item.image}
                            description={item.description}
                            price={item.Price}
                        />
                    ))}
                </View>
            </ScrollView>
        </>
    )
}

export default RestaurantScreen