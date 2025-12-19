import { Linking, SafeAreaView, StyleSheet, Text, View } from 'react-native';
import { styles } from './App.styles';
import { ProfileCard } from './components/profileCard/Profile';
// For android - react-native-safe-area-context
// import { SafeAreaView } from 'react-native-safe-area-context';

export default function App() {

  const onPressRedirect = (socialMedia) => {
    let url = "";
    switch (socialMedia) {
      case "linkedin":
        url = "https://www.linkedin.com"
        break;
      case "facebook":
        url = "https://www.facebook.com"
        break;
      case "github":
        url = "https://www.github.com"
        break;
      case "youtube":
        url = "https://www.youtube.com"
        break;
    
      default:
        break;
    }
    Linking.openURL(url);
  }

  return (
    <SafeAreaView style={styles.container}>
      <Text style={styles.title}>My First App</Text>

      <ProfileCard
        username={"MS Dhoni"}
        description={"Batsman | Wicket-keeper"}
        imgUrl={"https://images.news18.com/ibnlive/uploads/2024/10/ms-dhoni-csk-ipl-2025-2024-10-768f4f4630e2d64216ed2c73f82c179f-3x2.jpg"}
        redirect={onPressRedirect}
      />

      <ProfileCard
        username={"Virat Kohli"}
        description={"Batsman | Fielder"}
        imgUrl={"https://upload.wikimedia.org/wikipedia/commons/e/ef/Virat_Kohli_during_the_India_vs_Aus_4th_Test_match_at_Narendra_Modi_Stadium_on_09_March_2023.jpg"}
        redirect={onPressRedirect}
      />

    <ProfileCard
        username={"Yuvraj Singh"}
        description={"Batsman | All-rounder"}
        imgUrl={"https://static.toiimg.com/thumb/msid-102944476,imgsize-72298,width-400,resizemode-4/102944476.jpg"}
        redirect={onPressRedirect}
      />

   
    </SafeAreaView>
  );
}


// 1. Install expo-go app in mobile
// 2. npm start
// 3. scan QR code using expo-go application