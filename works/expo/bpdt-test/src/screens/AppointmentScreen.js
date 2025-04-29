import React, { useState, useEffect, useRef } from 'react';
import {
  StyleSheet,
  View,
  Text,
  TextInput,
  TouchableOpacity,
  ScrollView,
  Alert,
  Platform,
} from 'react-native';
import MapView, { Marker, PROVIDER_GOOGLE } from 'react-native-maps';
import * as Location from 'expo-location';
import { Ionicons } from '@expo/vector-icons';

export default function AppointmentScreen() {
  const [location, setLocation] = useState(null);
  const [appointments, setAppointments] = useState([]);
  const [newAppointment, setNewAppointment] = useState({
    title: '',
    date: '',
    time: '',
    location: '',
    notes: '',
  });
  const [selectedLocation, setSelectedLocation] = useState(null);
  const [errorMsg, setErrorMsg] = useState(null);
  const mapRef = useRef(null);

  useEffect(() => {
    (async () => {
      let { status } = await Location.requestForegroundPermissionsAsync();
      if (status !== 'granted') {
        setErrorMsg('위치 권한이 거부되었습니다.');
        return;
      }

      let location = await Location.getCurrentPositionAsync({});
      setLocation(location);
    })();
  }, []);

  const handleAddAppointment = () => {
    if (!newAppointment.title || !newAppointment.date || !newAppointment.time) {
      Alert.alert('오류', '제목, 날짜, 시간은 필수 입력사항입니다.');
      return;
    }

    const appointment = {
      ...newAppointment,
      id: Date.now().toString(),
      location: selectedLocation || location,
    };

    setAppointments([...appointments, appointment]);
    setNewAppointment({
      title: '',
      date: '',
      time: '',
      location: '',
      notes: '',
    });
    setSelectedLocation(null);
  };

  const handleMapPress = (e) => {
    setSelectedLocation({
      latitude: e.nativeEvent.coordinate.latitude,
      longitude: e.nativeEvent.coordinate.longitude,
    });
  };

  const goToUserLocation = async () => {
    const { status } = await Location.requestForegroundPermissionsAsync();
    if (status === 'granted') {
      const location = await Location.getCurrentPositionAsync({});
      mapRef.current.animateToRegion({
        latitude: location.coords.latitude,
        longitude: location.coords.longitude,
        latitudeDelta: 0.01,
        longitudeDelta: 0.01,
      }, 1000);
    }
  };

  const handleZoomIn = () => {
    const region = mapRef.current.getMapBoundaries();
    const newRegion = {
      ...region,
      latitudeDelta: region.latitudeDelta / 2,
      longitudeDelta: region.longitudeDelta / 2,
    };
    mapRef.current.animateToRegion(newRegion, 300);
  };

  const handleZoomOut = () => {
    const region = mapRef.current.getMapBoundaries();
    const newRegion = {
      ...region,
      latitudeDelta: region.latitudeDelta * 2,
      longitudeDelta: region.longitudeDelta * 2,
    };
    mapRef.current.animateToRegion(newRegion, 300);
  };

  return (
    <View style={styles.container}>
      <MapView
        ref={mapRef}
        style={styles.map}
        provider={PROVIDER_GOOGLE}
        initialRegion={{
          latitude: location?.coords.latitude || 37.5665,
          longitude: location?.coords.longitude || 126.9780,
          latitudeDelta: 0.0922,
          longitudeDelta: 0.0421,
        }}
        showsUserLocation={true}
        showsMyLocationButton={true}
        zoomEnabled={true}
        zoomControlEnabled={true}
        language="ko"
        onPress={handleMapPress}
      >
        {location && (
          <Marker
            coordinate={{
              latitude: location.coords.latitude,
              longitude: location.coords.longitude,
            }}
            title="현재 위치"
          />
        )}
        {selectedLocation && (
          <Marker
            coordinate={selectedLocation}
            title="선택된 위치"
            pinColor="red"
          />
        )}
      </MapView>

      {/* 위치 버튼 */}
      <TouchableOpacity
        style={[styles.button, styles.locationButton]}
        onPress={goToUserLocation}
      >
        <Ionicons name="locate" size={24} color="#007AFF" />
      </TouchableOpacity>

      {/* 줌 인 버튼 */}
      <TouchableOpacity
        style={[styles.button, styles.zoomInButton]}
        onPress={handleZoomIn}
      >
        <Ionicons name="add" size={24} color="#007AFF" />
      </TouchableOpacity>

      {/* 줌 아웃 버튼 */}
      <TouchableOpacity
        style={[styles.button, styles.zoomOutButton]}
        onPress={handleZoomOut}
      >
        <Ionicons name="remove" size={24} color="#007AFF" />
      </TouchableOpacity>

      <ScrollView style={styles.formContainer}>
        <Text style={styles.title}>새 약속 추가</Text>
        
        <TextInput
          style={styles.input}
          placeholder="약속 제목"
          value={newAppointment.title}
          onChangeText={(text) => setNewAppointment({ ...newAppointment, title: text })}
        />

        <TextInput
          style={styles.input}
          placeholder="날짜 (YYYY-MM-DD)"
          value={newAppointment.date}
          onChangeText={(text) => setNewAppointment({ ...newAppointment, date: text })}
        />

        <TextInput
          style={styles.input}
          placeholder="시간 (HH:MM)"
          value={newAppointment.time}
          onChangeText={(text) => setNewAppointment({ ...newAppointment, time: text })}
        />

        <TextInput
          style={styles.input}
          placeholder="메모"
          value={newAppointment.notes}
          onChangeText={(text) => setNewAppointment({ ...newAppointment, notes: text })}
          multiline
        />

        <TouchableOpacity
          style={styles.addButton}
          onPress={handleAddAppointment}
        >
          <Text style={styles.addButtonText}>약속 추가</Text>
        </TouchableOpacity>

        <Text style={styles.sectionTitle}>약속 목록</Text>
        {appointments.map((appointment) => (
          <View key={appointment.id} style={styles.appointmentItem}>
            <Text style={styles.appointmentTitle}>{appointment.title}</Text>
            <Text style={styles.appointmentDetail}>
              {appointment.date} {appointment.time}
            </Text>
            {appointment.notes && (
              <Text style={styles.appointmentNotes}>{appointment.notes}</Text>
            )}
          </View>
        ))}
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#fff',
  },
  map: {
    flex: 1,
  },
  formContainer: {
    flex: 1,
    padding: 20,
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 20,
  },
  input: {
    borderWidth: 1,
    borderColor: '#ddd',
    padding: 10,
    marginBottom: 15,
    borderRadius: 5,
  },
  addButton: {
    backgroundColor: '#007AFF',
    padding: 15,
    borderRadius: 5,
    alignItems: 'center',
    marginBottom: 20,
  },
  addButtonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: 'bold',
  },
  sectionTitle: {
    fontSize: 20,
    fontWeight: 'bold',
    marginBottom: 15,
  },
  appointmentItem: {
    backgroundColor: '#f8f8f8',
    padding: 15,
    borderRadius: 5,
    marginBottom: 10,
  },
  appointmentTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    marginBottom: 5,
  },
  appointmentDetail: {
    color: '#666',
    marginBottom: 5,
  },
  appointmentNotes: {
    color: '#888',
    fontStyle: 'italic',
  },
  button: {
    position: 'absolute',
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
  locationButton: {
    bottom: 20,
    right: 20,
  },
  zoomInButton: {
    top: 20,
    right: 20,
  },
  zoomOutButton: {
    top: 80,
    right: 20,
  },
}); 