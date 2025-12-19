import { View, Image, Text, TouchableOpacity } from "react-native"
import { styles } from "./Profile.styles"
// npm install @expo/vector-icons
import { FontAwesome } from "@expo/vector-icons"

export const ProfileCard = ({username, description, imgUrl, redirect}) => {
    return (
        <View style={styles.profileCard}>
            <View style={styles.header}>
                <View>
                    <Image 
                        style={styles.avatar}
                        source={{ uri : imgUrl }}
                    />
                </View>
                <View>
                    <Text style={styles.username}>{username}</Text>
                    <Text style={styles.desc}>{description}</Text>
                </View>
            </View>

            <View style={styles.socialMediaIcons}>
                <TouchableOpacity onPress={() => redirect("linkedin")}>
                    <FontAwesome
                        name="linkedin"
                        size={24}
                        color={"#0077B5"}
                    />
                </TouchableOpacity>

                <TouchableOpacity onPress={() => redirect("github")}>
                    <FontAwesome
                        name="github"
                        size={24}
                    />
                </TouchableOpacity>

                <TouchableOpacity onPress={() => redirect("facebook")}>
                    <FontAwesome
                        name="facebook"
                        size={24}
                        color={"#1877F2"}
                    />
                </TouchableOpacity>

                <TouchableOpacity onPress={() => redirect("youtube")}>
                    <FontAwesome
                        name="youtube"
                        size={24}
                        color={"#ff0000"}
                    />
                </TouchableOpacity>
            </View>
        </View>
    )
}