
function AppointmentScreen() {
	const [location, setLocation] = useState(null);
	const [errorMsg, setErrorMsg] = useState(null);
	const mapRef = React.useRef(null);

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
			/>
			<TouchableOpacity
				style={styles.locationButton}
				onPress={goToUserLocation}
			>
				<Ionicons name="locate" size={24} color="#007AFF" />
			</TouchableOpacity>
		</View>
	);
}
