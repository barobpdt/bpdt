import React, { useState, useRef, useEffect } from 'react';
import {
  View,
  Text,
  TextInput,
  TouchableOpacity,
  ScrollView,
  StyleSheet,
  KeyboardAvoidingView,
  Platform,
  Image,
} from 'react-native';
import { Ionicons } from '@expo/vector-icons';
import * as ImagePicker from 'expo-image-picker';
import { SafeAreaView } from 'react-native-safe-area-context';

const ChatScreen = () => {
  const [messages, setMessages] = useState([
    { id: 1, text: '안녕하세요! 무엇을 도와드릴까요?', time: '10:00', isReceived: true },
  ]);
  const [inputText, setInputText] = useState('');
  const [showEmojiPicker, setShowEmojiPicker] = useState(false);
  const [selectedImages, setSelectedImages] = useState([]);
  const [showUploadScreen, setShowUploadScreen] = useState(false);
  const scrollViewRef = useRef();

  const emojis = {
    faces: ['😀', '😃', '😄', '😁', '😅', '😂', '🤣', '😊', '😇', '🙂', '🙃', '😉', '😌', '😍', '🥰'],
    animals: ['🐶', '🐱', '🐭', '🐹', '🐰', '🦊', '🐻', '🐼', '🐨', '🐯', '🦁', '🐮', '🐷', '🐸', '🐵'],
    food: ['🍎', '🍐', '🍊', '🍋', '🍌', '🍉', '🍇', '🍓', '🍈', '🍒', '🍑', '🥭', '🍍', '🥥', '🥝'],
  };

  const pickImage = async () => {
    const result = await ImagePicker.launchImageLibraryAsync({
      mediaTypes: ImagePicker.MediaTypeOptions.Images,
      allowsMultipleSelection: true,
      quality: 1,
    });

    if (!result.canceled) {
      setSelectedImages(result.assets);
      setShowUploadScreen(true);
    }
  };

  const sendMessage = () => {
    if (inputText.trim() === '') return;

    const newMessage = {
      id: messages.length + 1,
      text: inputText,
      time: new Date().toLocaleTimeString('ko-KR', { hour: '2-digit', minute: '2-digit' }),
      isReceived: false,
    };

    setMessages([...messages, newMessage]);
    setInputText('');
    scrollViewRef.current?.scrollToEnd({ animated: true });
  };

  const renderMessage = (message) => (
    <View
      key={message.id}
      style={[
        styles.messageContainer,
        message.isReceived ? styles.receivedMessage : styles.sentMessage,
      ]}
    >
      <View style={styles.messageContent}>
        <Text style={styles.messageText}>{message.text}</Text>
        <Text style={styles.messageTime}>{message.time}</Text>
      </View>
    </View>
  );

  const renderEmojiPicker = () => (
    <View style={styles.emojiPicker}>
      {Object.entries(emojis).map(([category, emojiList]) => (
        <View key={category} style={styles.emojiCategory}>
          <Text style={styles.emojiCategoryTitle}>{category}</Text>
          <View style={styles.emojiList}>
            {emojiList.map((emoji, index) => (
              <TouchableOpacity
                key={index}
                style={styles.emojiButton}
                onPress={() => {
                  setInputText(inputText + emoji);
                  setShowEmojiPicker(false);
                }}
              >
                <Text style={styles.emoji}>{emoji}</Text>
              </TouchableOpacity>
            ))}
          </View>
        </View>
      ))}
    </View>
  );

  const renderUploadScreen = () => (
    <View style={styles.uploadContainer}>
      <View style={styles.uploadArea}>
        <Text style={styles.uploadIcon}>📁</Text>
        <Text style={styles.uploadText}>파일을 선택하여 업로드하세요</Text>
        <TouchableOpacity style={styles.uploadButton} onPress={pickImage}>
          <Text style={styles.uploadButtonText}>파일 선택</Text>
        </TouchableOpacity>
      </View>

      {selectedImages.length > 0 && (
        <View style={styles.fileList}>
          {selectedImages.map((image, index) => (
            <View key={index} style={styles.fileItem}>
              <Image source={{ uri: image.uri }} style={styles.filePreview} />
            </View>
          ))}
        </View>
      )}

      <View style={styles.buttonGroup}>
        <TouchableOpacity style={styles.actionButton} onPress={() => {}}>
          <Text style={styles.buttonText}>업로드</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={styles.actionButton}
          onPress={() => {
            setSelectedImages([]);
            setShowUploadScreen(false);
          }}
        >
          <Text style={styles.buttonText}>초기화</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={styles.actionButton}
          onPress={() => setShowUploadScreen(false)}
        >
          <Text style={styles.buttonText}>취소</Text>
        </TouchableOpacity>
      </View>
    </View>
  );

  return (
    <SafeAreaView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.headerTitle}>채팅 메시지</Text>
        <View style={styles.userInfo}>
          <Text style={styles.userName}>사용자</Text>
          <View style={styles.status} />
        </View>
      </View>

      {showUploadScreen ? (
        renderUploadScreen()
      ) : (
        <KeyboardAvoidingView
          behavior={Platform.OS === 'ios' ? 'padding' : 'height'}
          style={styles.chatContainer}
        >
          <ScrollView
            ref={scrollViewRef}
            style={styles.messagesContainer}
            contentContainerStyle={styles.messagesContent}
          >
            {messages.map(renderMessage)}
          </ScrollView>

          {showEmojiPicker && renderEmojiPicker()}

          <View style={styles.inputContainer}>
            <View style={styles.toolbar}>
              <TouchableOpacity
                style={styles.toolbarButton}
                onPress={() => setShowEmojiPicker(!showEmojiPicker)}
              >
                <Ionicons name="happy-outline" size={24} color="#666" />
              </TouchableOpacity>
              <TouchableOpacity style={styles.toolbarButton} onPress={pickImage}>
                <Ionicons name="attach-outline" size={24} color="#666" />
              </TouchableOpacity>
            </View>

            <View style={styles.inputWrapper}>
              <TextInput
                style={styles.input}
                placeholder="메시지를 입력하세요..."
                value={inputText}
                onChangeText={setInputText}
                multiline
              />
              <TouchableOpacity style={styles.sendButton} onPress={sendMessage}>
                <Ionicons name="send" size={24} color="#fff" />
              </TouchableOpacity>
            </View>
          </View>
        </KeyboardAvoidingView>
      )}
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#fff',
  },
  header: {
    padding: 16,
    borderBottomWidth: 1,
    borderBottomColor: '#eee',
  },
  headerTitle: {
    fontSize: 20,
    fontWeight: 'bold',
    marginBottom: 8,
  },
  userInfo: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  userName: {
    fontSize: 16,
    marginRight: 8,
  },
  status: {
    width: 10,
    height: 10,
    borderRadius: 5,
    backgroundColor: '#4CAF50',
  },
  chatContainer: {
    flex: 1,
  },
  messagesContainer: {
    flex: 1,
  },
  messagesContent: {
    padding: 16,
  },
  messageContainer: {
    maxWidth: '80%',
    marginBottom: 16,
  },
  receivedMessage: {
    alignSelf: 'flex-start',
  },
  sentMessage: {
    alignSelf: 'flex-end',
  },
  messageContent: {
    backgroundColor: '#f0f0f0',
    padding: 12,
    borderRadius: 16,
  },
  messageText: {
    fontSize: 16,
  },
  messageTime: {
    fontSize: 12,
    color: '#666',
    marginTop: 4,
    alignSelf: 'flex-end',
  },
  inputContainer: {
    padding: 16,
    borderTopWidth: 1,
    borderTopColor: '#eee',
  },
  toolbar: {
    flexDirection: 'row',
    marginBottom: 8,
  },
  toolbarButton: {
    marginRight: 16,
  },
  inputWrapper: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  input: {
    flex: 1,
    borderWidth: 1,
    borderColor: '#ddd',
    borderRadius: 20,
    paddingHorizontal: 16,
    paddingVertical: 8,
    maxHeight: 100,
  },
  sendButton: {
    backgroundColor: '#007AFF',
    width: 40,
    height: 40,
    borderRadius: 20,
    justifyContent: 'center',
    alignItems: 'center',
    marginLeft: 8,
  },
  emojiPicker: {
    backgroundColor: '#fff',
    padding: 16,
    borderTopWidth: 1,
    borderTopColor: '#eee',
  },
  emojiCategory: {
    marginBottom: 16,
  },
  emojiCategoryTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    marginBottom: 8,
  },
  emojiList: {
    flexDirection: 'row',
    flexWrap: 'wrap',
  },
  emojiButton: {
    padding: 8,
  },
  emoji: {
    fontSize: 24,
  },
  uploadContainer: {
    flex: 1,
    padding: 16,
  },
  uploadArea: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    borderWidth: 2,
    borderColor: '#ddd',
    borderStyle: 'dashed',
    borderRadius: 8,
    padding: 32,
  },
  uploadIcon: {
    fontSize: 48,
    marginBottom: 16,
  },
  uploadText: {
    fontSize: 16,
    color: '#666',
    marginBottom: 16,
  },
  uploadButton: {
    backgroundColor: '#007AFF',
    paddingHorizontal: 16,
    paddingVertical: 8,
    borderRadius: 4,
  },
  uploadButtonText: {
    color: '#fff',
    fontSize: 16,
  },
  fileList: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    marginTop: 16,
  },
  fileItem: {
    width: 100,
    height: 100,
    margin: 8,
  },
  filePreview: {
    width: '100%',
    height: '100%',
    borderRadius: 4,
  },
  buttonGroup: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    marginTop: 16,
  },
  actionButton: {
    backgroundColor: '#007AFF',
    paddingHorizontal: 16,
    paddingVertical: 8,
    borderRadius: 4,
  },
  buttonText: {
    color: '#fff',
    fontSize: 16,
  },
});

export default ChatScreen; 