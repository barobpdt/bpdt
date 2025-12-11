import { View, Text, ScrollView } from 'react-native'
import React, { useEffect, useState } from 'react'
import { ArrowDownRightIcon, ArrowRightIcon } from 'react-native-heroicons/solid'
import RestaurantsCards from './RestaurantsCards'
import sanityClient from '../sanity.js'


const FeatureRows = ({ id, title, description, FeatureCategories }) => {
    const [restaurant, setrestaurant] = useState([]);
    useEffect(() => {
        sanityClient.fetch(`*[_type == 'feature' && _id == $id] {
  ...,
  restaurants[]-> {
    ...,
    dishes[]->,
    type->{
    name
    
    }
  }
}[0]`, { id }).then((data) => {
           

            setrestaurant(data?.restaurants)
        })
    }, [])
    return (
        <View>
            <View className='flex-row items-center justify-between mt-4 px-4'>
                <Text className="font-bold text-lg">
                    {title}
                </Text>
                <ArrowRightIcon color="#16C47F" />
            </View>
            <Text className='text-xs text-gray-400 px-4 '>{description}</Text>
            <ScrollView
                horizontal
                showsHorizontalScrollIndicator={false}
                contentContainerStyle={{
                    paddingHorizontal: 15,
                }}
                className='pt-3'
            >
                {restaurant?.map((item) => (
                    <RestaurantsCards
                        key={item._id}
                        id={item._id}
                        img={item.image}
                        title={item.name}
                        description={item.description}
                        genre={item.name}
                        long={item.long}
                        lat={item.lat}
                        dishes={item.dishes}
                        address={item.address}
                        rating={item.rating}
                    />
                ))}


            </ScrollView>

        </View>
    )
}

export default FeatureRows