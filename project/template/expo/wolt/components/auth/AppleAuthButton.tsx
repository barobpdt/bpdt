import Ionicons from '@expo/vector-icons/Ionicons';
import { StyleSheet, Text, TouchableOpacity } from 'react-native';

const AppleAuthButton = () => {
  return (
    <TouchableOpacity style={styles.appleButton}>
      <Ionicons name="logo-google" size={18} color={'#fff'} />
      <Text style={styles.appleButtonText}>Sign in with Apple</Text>
    </TouchableOpacity>
  );
};
const styles = StyleSheet.create({
  appleButton: {
    backgroundColor: '#000',
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    paddingVertical: 17,
    borderRadius: 12,
    gap: 4,
  },
  appleButtonText: {
    color: '#fff',
    fontSize: 18,
    fontWeight: '600',
  },
});
export default AppleAuthButton;
