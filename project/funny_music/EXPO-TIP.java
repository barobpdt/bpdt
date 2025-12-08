##> 인증
	npx create-expo-app my-clerk-app 등으로 Expo 앱 생성 후, 
	npm install @clerk/clerk-expo를 통해 Clerk SDK 설치.

	Clerk Provider 설정:
	Expo Router를 사용한다면, app/_layout.tsx 파일에서 ClerkProvider로 전체 앱을 감쌉니다.
	publishableKey와 tokenCache 프롭스를 설정하고, expo-secure-store를 사용해 토큰을 안전하게 저장합니다.

	인증 UI 구현:
	사인인/사인업 화면: @clerk/clerk-expo에서 제공하는 SignIn 컴포넌트나 useSignIn 훅을 사용해 이메일/비밀번호 기반 로그인을 구현.
	소셜 로그인: 구글, 깃허브 등 소셜 로그인 버튼을 추가하고, Clerk가 제공하는 OAuth 흐름을 통해 처리.

	2 설치
	npx create-expo-app@latest clerkApp —example tabs@49
	cd clerkApp

	# Install Clerk and Expo Secure Store for token storage
	npm install @clerk/clerk-expo
	npx expo install expo-secure-store

	# Simple loading spinner
	npm install react-native-loading-spinner-overlay

	# Start the app
	npx expo

	2 .env에 추가
	EXPO_PUBLIC_CLERK_PUBLISHABLE_KEY=pk_test_YOURKEY

	3. 인증 화면구현
	import { useEffect } from 'react';
	import * as SecureStore from 'expo-secure-store';
	import { ClerkProvider, useAuth } from '@clerk/clerk-expo';
	import { Slot, useRouter, useSegments } from 'expo-router';

	const CLERK_PUBLISHABLE_KEY = process.env.EXPO_PUBLIC_CLERK_PUBLISHABLE_KEY;
	// Cache the Clerk JWT
	const tokenCache = {
		async getToken(key: string) {
			try {
				return SecureStore.getItemAsync(key);
			} catch (err) {
				return null;
			}
		},
		async saveToken(key: string, value: string) {
			try {
				return SecureStore.setItemAsync(key, value);
			} catch (err) {
				return;
			}
		}
	};
	const RootLayoutNav = () => {
		return (
			<ClerkProvider publishableKey={CLERK_PUBLISHABLE_KEY!} tokenCache={tokenCache}>
				<Slot />
			</ClerkProvider>
		);
	};
	export default RootLayoutNav;

