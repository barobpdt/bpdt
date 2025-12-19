import { StyleSheet } from "react-native";

export const styles = StyleSheet.create({
    profileCard: {
        // borderWidth: 1,
        // borderColor: "red",
        // borderStyle: "solid",
        padding: 10,
        width: "90%",
        marginBottom: 40,
        backgroundColor: "#fff",
        shadowColor: "#000",
        shadowOpacity: 0.3,
        shadowOffset: {
            width: 1,
            height: 3
        }
    },
    header: {
        flexDirection: "row",
        alignItems: "center",
        justifyContent: "space-evenly"
    },
    avatar: {
        height: 80,
        width: 80,
        borderRadius: "50%",
        objectFit: "cover"
    },
    username: {
        fontWeight: "bold",
        fontSize: 20
    },
    desc: {
        fontSize: 16
    },
    socialMediaIcons : {
        flexDirection: "row",
        justifyContent: "space-evenly",
        marginTop: 20
    }
})