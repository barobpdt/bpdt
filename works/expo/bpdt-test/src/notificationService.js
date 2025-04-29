import * as Notifications from 'expo-notifications';
import * as Device from 'expo-device';
import Constants from 'expo-constants';
import { Platform } from 'react-native';

// 알림 핸들러 설정
Notifications.setNotificationHandler({
  handleNotification: async () => ({
    shouldShowAlert: true,
    shouldPlaySound: true,
    shouldSetBadge: true,
  }),
});

// Firebase 토큰을 받는 함수
export async function getFirebaseToken() {
  try {
    // Expo Push Token을 먼저 받습니다
    const expoPushToken = await registerForPushNotificationsAsync();
    
    if (!expoPushToken) {
      console.log('Failed to get Expo push token');
      return null;
    }

    // Expo Push Token을 Firebase 토큰으로 변환
    const response = await fetch('https://exp.host/--/api/v2/push/getExpoPushToken', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        deviceId: Device.deviceName,
        experienceId: '@your-username/fcm-push-app', // Expo 프로젝트의 experience ID
        appId: 'com.yourcompany.fcmpushapp', // app.json의 android.package와 동일
        deviceToken: expoPushToken.data,
        type: 'fcm',
      }),
    });

    const data = await response.json();
    if (data.data) {
      console.log('Firebase Token:', data.data);
      return data.data;
    } else {
      console.log('Failed to get Firebase token:', data);
      return null;
    }
  } catch (error) {
    console.error('Error getting Firebase token:', error);
    return null;
  }
}

// 푸시 알림 토큰 등록
export async function registerForPushNotificationsAsync() {
  let token;

  if (Device.isDevice) {
    const { status: existingStatus } = await Notifications.getPermissionsAsync();
    let finalStatus = existingStatus;
    
    if (existingStatus !== 'granted') {
      const { status } = await Notifications.requestPermissionsAsync();
      finalStatus = status;
    }
    
    if (finalStatus !== 'granted') {
      console.log('Failed to get push token for push notification!');
      return;
    }

    token = await Notifications.getExpoPushTokenAsync({
      projectId: Constants.expoConfig.extra.eas.projectId,
    });
  } else {
    console.log('Must use physical device for Push Notifications');
  }

  return token;
}

// 로컬 알림 전송
export async function sendLocalNotification(title, body, data = {}) {
  await Notifications.scheduleNotificationAsync({
    content: {
      title,
      body,
      data,
    },
    trigger: { seconds: 2 },
  });
}

// 알림 리스너 설정
export function setupNotificationListeners(onNotificationReceived, onNotificationResponse) {
  const notificationListener = Notifications.addNotificationReceivedListener(notification => {
    if (onNotificationReceived) {
      onNotificationReceived(notification);
    }
  });

  const responseListener = Notifications.addNotificationResponseReceivedListener(response => {
    if (onNotificationResponse) {
      onNotificationResponse(response);
    }
  });

  return () => {
    Notifications.removeNotificationSubscription(notificationListener);
    Notifications.removeNotificationSubscription(responseListener);
  };
}

// 배지 카운트 설정
export async function setBadgeCount(count) {
  await Notifications.setBadgeCountAsync(count);
}

// 알림 채널 설정 (Android)
export async function configureNotificationChannel() {
  if (Platform.OS === 'android') {
    await Notifications.setNotificationChannelAsync('default', {
      name: 'default',
      importance: Notifications.AndroidImportance.MAX,
      vibrationPattern: [0, 250, 250, 250],
      lightColor: '#FF231F7C',
    });
  }
} 