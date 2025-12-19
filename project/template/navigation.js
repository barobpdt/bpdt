import { Suspense, lazy } from 'react';

const HomeScreen = lazy(() => import('./HomeScreen'));
const ProfileScreen = lazy(() => import('./ProfileScreen'));

function MyStack() {
  return (
    <Stack.Navigator
      screenLayout={({ children }) => (
        <Suspense fallback={<Loading />}>{children}</Suspense>
      )}
    >
      <Stack.Screen name="Home" component={HomeScreen} />
      <Stack.Screen name="Profile" component={ProfileScreen} />
    </Stack.Navigator>
  );
}