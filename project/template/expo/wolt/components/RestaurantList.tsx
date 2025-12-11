import { Colors } from '@/constants/theme';
import { useRestaurants } from '@/hooks/useRestaurants';
import Ionicons from '@expo/vector-icons/Ionicons';
import { Link } from 'expo-router';
import { ActivityIndicator, Image, StyleSheet, Text, TouchableOpacity, View } from 'react-native';
const RestaurantList = () => {
  const { data: restaurants, isLoading, error } = useRestaurants();

  if (isLoading) {
    return (
      <View>
        <ActivityIndicator size={'large'} color={Colors.secondary} />
      </View>
    );
  }

  if (error) {
    return (
      <View style={{ padding: 16, alignItems: 'center' }}>
        <Text style={{ color: Colors.dark, marginBottom: 8 }}>Failed to load restaurants</Text>
        <Text style={{ color: Colors.muted }}>
          {error instanceof Error ? error.message : 'Please try again later'}
        </Text>
      </View>
    );
  }

  return (
    <>
      {restaurants?.map((item) => (
        <View key={item.id}>
          <Link href={`/(modal)/(restaurant)/${item.id}`} asChild>
            <TouchableOpacity style={styles.card}>
              <Image source={item.image!} style={styles.image} />
              <View style={styles.info}>
                <Text style={styles.name}>{item.name}</Text>
                <Text style={styles.description} numberOfLines={2}>
                  {item.description}
                </Text>
              </View>
              <View style={styles.metadata}>
                <Ionicons name="bicycle-outline" size={16} color={Colors.muted} />
                <Text style={styles.metadataText}>€{item.deliveryFee.toFixed(2)}</Text>
                <Text style={styles.dot}>•</Text>
                <Text style={styles.metadataText}>€€€€</Text>
                <Text style={styles.dot}>•</Text>
                <Ionicons name="happy-outline" size={16} color={Colors.muted} />
              </View>
            </TouchableOpacity>
          </Link>
        </View>
      ))}
    </>
  );
};

const styles = StyleSheet.create({
  card: {
    margin: 16,
    borderRadius: 12,
    borderWidth: StyleSheet.hairlineWidth,
    borderColor: Colors.light,
    overflow: 'hidden',
    boxShadow: '0px 4px 2px -2px rgba(0,0,0, 0.2)',
    elevation: 2,
  },
  image: {
    width: '100%',
    height: 180,
  },
  info: {
    padding: 12,
  },
  name: {
    fontSize: 16,
    fontWeight: '600',
    marginBottom: 4,
  },
  description: {
    fontSize: 14,
    color: Colors.muted,
  },
  metadata: {
    borderTopColor: Colors.light,
    borderTopWidth: StyleSheet.hairlineWidth,
    flexDirection: 'row',
    alignItems: 'center',
    gap: 4,
    padding: 10,
  },
  metadataText: {
    fontSize: 13,
    color: Colors.muted,
  },
  dot: {
    color: '#999',
    fontSize: 13,
  },
});
export default RestaurantList;
