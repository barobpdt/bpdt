import { configureStore } from '@reduxjs/toolkit'
import CartReducer from './features/CartSlice'
import RestaurantReducer from './features/restaurant'
export const store = configureStore({
  reducer: {
    cart:CartReducer,
    restaurant:RestaurantReducer
  },
})