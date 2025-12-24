## config
react-native-web

##> store
// 스토어 정의 (예시)
import { create } from 'zustand';
const useMyStore = create((set) => ({
  valueA: 0,
  valueB: 'hello',
  setValueA: (val) => set({ valueA: val }),
  setValueB: (val) => set({ valueB: val }),
}));

##1
import { useMyStore } from './useMyStore';
import shallow from 'zustand/shallow';
const ComponentA = () => {
  // state.valueA가 변경될 때만 이 컴포넌트가 리렌더링됩니다.
  const valueA = useMyStore((state) => state.valueA); 

  return <div>Value A: {valueA}</div>;
};

## 2
import { useMyStore } from './useMyStore';
import { shallow } from 'zustand/shallow';
const MyComponent = () => {
  // useShallow를 사용하여 A 또는 B 중 하나라도 바뀌었을 때만 리렌더링
  const { valueA, valueB } = useMyStore(
    (state) => ({ valueA: state.valueA, valueB: state.valueB }),
    shallow 
  );
  return (
    <div>
      <p>Value A: {valueA}</p>
      <p>Value B: {valueB}</p>
    </div>
  );
};

##3
// 컴포넌트 내부나 useEffect 등에서 사용
useEffect(() => {
  const unsubscribe = useMyStore.subscribe(
    (state) => state.valueA,
    (currentValue, previousValue) => {
      // valueA가 변경될 때만 실행되는 로직 (컴포넌트 리렌더링 X)
      console.log('valueA가 변경되었습니다:', currentValue);
    }
  );
  return () => unsubscribe();
}, []);

##1
import { StoreApi, UseBoundStore } from "zustand";
import { shallow } from "zustand/shallow";
export const useShallow = <T, K extends keyof T>(
  store: UseBoundStore<StoreApi<T>>,
  keys: K[]
): Pick<T, K> => {
  return store((state) => {
    const result = {} as { [K in keyof T]: T[K] };
    keys.forEach((key) => {
      result[key] = state[key];
    });
    return result;
  }, shallow);
};
##2
import { create } from "zustand";
import { useShallow } from "../hooks/useShallow";
interface AppStoreStates {
  a: number;
  b: number;
  setA: (a: number) => void;
  setB: (b: number) => void;
}
export const appStore = create<AppStoreStates>((set) => ({
  a: 0,
  b: 0,
  setA: (a: number) => set({ a }),
  setB: (b: number) => set({ b }),
}));
export const useAppStore = <T extends keyof AppStoreStates>(keys: T[]) => {
  return useShallow(appStore, keys);
};

##3
const A = () => {
  const { a, setA } = useAppStore(["a", "setA"]);
  console.log("a rendering");
  return (
    <>
      <div>{a}</div>
      <button onClick={() => setA(a + 1)}>set A</button>
    </>
  );
};

const B = () => {
  const { b, setB } = useAppStore(["b", "setB"]);
  console.log("b rendering");
  return (
    <>
      <div>{b}</div>
      <button onClick={() => setB(b + 1)}>set A</button>
    </>
  );
};


##> route
// App.js 또는 index.js
import { BrowserRouter, Routes, Route, Link } from 'react-router-dom';
import Home from './Home';
import About from './About';
function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<Home />} />
        <Route path="/about" element={<About />} />
        {/* 동적 라우트 예시 */}
        <Route path="/users/:userId" element={<UserProfile />} />
      </Routes>
      <nav>
        <Link to="/">홈</Link> | <Link to="/about">소개</Link>
      </nav>
    </BrowserRouter>
  );
}
##> route hook 훅
useNavigate(): 프로그램적으로 페이지를 이동시킬 때 사용합니다 (예: 로그인 성공 후 메인 페이지로 이동).
useParams(): URL 파라미터(/:userId 등) 값을 가져올 때 사용합니다.
useSearchParams(): URL 쿼리 스트링(?key=value)을 다룰 때 사용합니다. 

ex)
[페이지]
import { useParams } from "react-router-dom";
const UserProfile = () => {
  const { username } = useParams();
  return <h2>User Profile: {username}</h2>;
};
[호출]
<Routes>
  <Route path="/user/:username" element={<UserProfile />} />
</Routes>

[스크립트 페이지 이동]
import { useNavigate } from "react-router-dom";
const Home = () => {
  const navigate = useNavigate();
  return (
    <div>
      <h1>Home Page</h1>
      <button onClick={() => navigate("/about")}>Go to About</button>
    </div>
  );
};
export default Home;


##> navigation static
import { Suspense, lazy } from 'react';
const MyStack = createNativeStackNavigator({
  screenLayout: ({ children }) => (
    <Suspense fallback={<Loading />}>{children}</Suspense>
  ),
  screens: {
    Home: {
      component: lazy(() => import('./HomeScreen')),
    },
    Profile: {
      component: lazy(() => import('./ProfileScreen')),
    },
  },
});
##> dynamic

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

##> Vercel
To handle redirects on Vercel, add the following in the vercel.json file at the root of your project:

{
  "rewrites": [{ "source": "/(.*)", "destination": "/index.html" }]
}
