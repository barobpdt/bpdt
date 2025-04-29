import React, { useState, useEffect } from 'react';
import { NavigationContainer } from '@react-navigation/native';
import { createBottomTabNavigator } from '@react-navigation/bottom-tabs';
import { Ionicons } from '@expo/vector-icons';
import { StyleSheet, View, Text, TouchableOpacity, FlatList, TextInput } from 'react-native';
import * as Notifications from 'expo-notifications';
import * as Device from 'expo-device';
import { initializeApp } from 'firebase/app';
import { getMessaging, getToken, onMessage } from 'firebase/messaging';
import AppointmentScreen from './src/screens/AppointmentScreen';

const Tab = createBottomTabNavigator();

// Firebase 설정
const firebaseConfig = {
	apiKey: "AIzaSyBJebWQypVy5jFIGeQR2nl-RCqjEiMuig4",
	projectId: "jkjapp-b27d7",
	storageBucket: "jkjapp-b27d7.firebasestorage.app",
	messagingSenderId: "108149939140",
	appId: "1:108149939140:android:78919625fe63bbb9a22291"
};

// Firebase 초기화
const app = initializeApp(firebaseConfig);
const messaging = getMessaging(app);

// 알림 설정
Notifications.setNotificationHandler({
	handleNotification: async () => ({
		shouldShowAlert: true,
		shouldPlaySound: true,
		shouldSetBadge: true,
	}),
});

// 백그라운드 알림 처리
Notifications.addNotificationResponseReceivedListener(response => {
	console.log('알림 클릭:', response);
	// 여기에 알림 클릭 시 처리할 로직 추가
});

function ChatScreen() {
	const [messages, setMessages] = useState([]);
	const [newMessage, setNewMessage] = useState('');

	const sendMessage = () => {
		if (newMessage.trim()) {
			setMessages([...messages, { id: Date.now(), text: newMessage, sender: 'user' }]);
			setNewMessage('');
		}
	};

	return (
		<View style={styles.chatContainer}>
			<FlatList
				data={messages}
				keyExtractor={(item) => item.id.toString()}
				renderItem={({ item }) => (
					<View style={styles.messageContainer}>
						<Text style={styles.messageText}>{item.text}</Text>
					</View>
				)}
			/>
			<View style={styles.inputContainer}>
				<TextInput
					style={styles.input}
					value={newMessage}
					onChangeText={setNewMessage}
					placeholder="메시지를 입력하세요"
				/>
				<TouchableOpacity style={styles.sendButton} onPress={sendMessage}>
					<Ionicons name="send" size={24} color="#007AFF" />
				</TouchableOpacity>
			</View>
		</View>
	);
}

export default function App() {
	const [expoPushToken, setExpoPushToken] = useState('');

	useEffect(() => {
		registerForPushNotificationsAsync();
		setupFCM();
	}, []);

	const registerForPushNotificationsAsync = async () => {
		let token;
		if (Device.isDevice) {
			const { status: existingStatus } = await Notifications.getPermissionsAsync();
			let finalStatus = existingStatus;
			if (existingStatus !== 'granted') {
				const { status } = await Notifications.requestPermissionsAsync();
				finalStatus = status;
			}
			if (finalStatus !== 'granted') {
				alert('푸시 알림 권한이 거부되었습니다!');
				return;
			}
			token = (await Notifications.getExpoPushTokenAsync()).data;
			setExpoPushToken(token);
		} else {
			alert('푸시 알림은 실제 기기에서만 작동합니다!');
		}
	};

	const setupFCM = async () => {
		try {
			const token = await getToken(messaging);
			console.log('FCM Token:', token);

			// 백그라운드 메시지 처리
			onMessage(messaging, (payload) => {
				console.log('백그라운드 메시지 수신:', payload);
				// 알림 표시
				Notifications.scheduleNotificationAsync({
					content: {
						title: payload.notification?.title || '새 메시지',
						body: payload.notification?.body || '메시지가 도착했습니다',
					},
					trigger: null,
				});
			});
		} catch (error) {
			console.error('FCM 설정 중 오류:', error);
		}
	};

	return (
		<NavigationContainer>
			<Tab.Navigator
				screenOptions={({ route }) => ({
					tabBarIcon: ({ focused, color, size }) => {
						let iconName;
						if (route.name === '약속 관리') {
							iconName = focused ? 'calendar' : 'calendar-outline';
						} else if (route.name === '채팅') {
							iconName = focused ? 'chatbubbles' : 'chatbubbles-outline';
						}
						return <Ionicons name={iconName} size={size} color={color} />;
					},
					tabBarActiveTintColor: '#007AFF',
					tabBarInactiveTintColor: 'gray',
				})}
			>
				<Tab.Screen 
					name="약속 관리" 
					component={AppointmentScreen}
					options={{
						headerShown: false,
					}}
				/>
				<Tab.Screen 
					name="채팅" 
					component={ChatScreen}
					options={{
						headerShown: false,
					}}
				/>
			</Tab.Navigator>
		</NavigationContainer>
	);
}

const styles = StyleSheet.create({
	chatContainer: {
		flex: 1,
		backgroundColor: '#fff',
	},
	messageContainer: {
		padding: 10,
		margin: 5,
		backgroundColor: '#f0f0f0',
		borderRadius: 10,
		maxWidth: '80%',
		alignSelf: 'flex-start',
	},
	messageText: {
		fontSize: 16,
	},
	inputContainer: {
		flexDirection: 'row',
		padding: 10,
		borderTopWidth: 1,
		borderTopColor: '#eee',
	},
	input: {
		flex: 1,
		borderWidth: 1,
		borderColor: '#ddd',
		borderRadius: 20,
		padding: 10,
		marginRight: 10,
	},
	sendButton: {
		justifyContent: 'center',
		alignItems: 'center',
		width: 40,
		height: 40,
		borderRadius: 20,
		backgroundColor: '#f0f0f0',
	},
	container: {
		flex: 1,
	},
	map: {
		flex: 1,
	},
	locationButton: {
		position: 'absolute',
		bottom: 20,
		right: 20,
		backgroundColor: 'white',
		padding: 10,
		borderRadius: 30,
		shadowColor: '#000',
		shadowOffset: {
			width: 0,
			height: 2,
		},
		shadowOpacity: 0.25,
		shadowRadius: 3.84,
		elevation: 5,
	},
}); 