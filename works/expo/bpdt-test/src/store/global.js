import { create } from 'zustand';

const useGlobalStore = create((set) => ({
  // 사용자 정보
  user: {
    id: null,
    name: '사용자',
    isOnline: true,
  },

  // 채팅 관련 상태
  chat: {
    currentRoom: null,
    unreadCount: 0,
  },

  // 사용자 정보 업데이트
  setUser: (userData) => set((state) => ({
    user: { ...state.user, ...userData }
  })),

  // 채팅방 변경
  setCurrentRoom: (roomId) => set((state) => ({
    chat: { ...state.chat, currentRoom: roomId }
  })),

  // 읽지 않은 메시지 수 업데이트
  setUnreadCount: (count) => set((state) => ({
    chat: { ...state.chat, unreadCount: count }
  })),

  // 온라인 상태 토글
  toggleOnlineStatus: () => set((state) => ({
    user: { ...state.user, isOnline: !state.user.isOnline }
  })),
}));

export default useGlobalStore; 