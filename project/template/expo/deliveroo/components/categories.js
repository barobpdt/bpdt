import { View, Text, ScrollView } from 'react-native'
import CardCategories from './CardCategories'
import { useEffect, useState } from 'react'
// import { sanityClient } from '@sanity/client';
import sanityClient from '../sanity.js'
const Categories = () => {
  const [categories, setCategories] = useState();
  useEffect(() => {
    sanityClient.fetch(`
      *[_type == 'category'] {
...,
}
      `).then((data) => {
        console.log(data,'data');
      setCategories(data)
    })
  }, [])
  console.log(categories,'categories');
  
  return (
    <ScrollView
      horizontal
      showsHorizontalScrollIndicator={false}
      contentContainerStyle={{
        paddingHorizontal: 0,
        paddingTop: 10,

      }}>
      {/* CardCategories */}
      {categories?.map((item) => (
        <CardCategories key={item._id} imgUrl={item.image} title={item.name} />
      ))}


    </ScrollView>
  )
}

export default Categories