import { create } from 'zustand';
import axios from 'axios';

const BASE_URL = 'https://api.example.com'; // Replace with your API URL

// Create the store
const useBasicStore = create((set, get) => ({
  items: [], // Initial state

  // Action to fetch items from an API
  fetchItems: async () => {
    try {
      const response = await axios.get(`${BASE_URL}/items`); // Fetching items
      const data = response.data;
      set({ items: data });
    } catch (error) {
      console.error('Failed to fetch items:', error);
    }
  },

  // Action to add an item
  addItem: (item) =>
    set((state) => ({
      items: [...state.items, item],
    })),

  // Action to remove an item
  removeItem: (id) =>
    set((state) => ({
      items: state.items.filter((item) => item.id !== id),
    })),

  // Action to get the count of items
  getItemCount: () => get().items.length,

  // Optional: Clear all items
  clearItems: () => set({ items: [] }),

  // Optional: Update an item by id
  updateItem: (id, updatedItem) =>
    set((state) => ({
      items: state.items.map((item) => (item.id === id ? updatedItem : item)),
    })),
}));

const { items, addItem, removeItem, getItemCount } = useBasicStore();



export const fetchItemsService = async (id) => {
	const BASE_URL = 'https://jsonplaceholder.typicode.com'; // Example URL
  try {
    // Fetching items using a sample placeholder API (fetching a post as an example)
    const response = await axios.post(`${BASE_URL}/posts`, { userId: id });

    // Mock token and items data, adapt based on your actual response structure
    const { data: items } = response;

    // Update store with fetched items
    const { addItem } = useBasicStore.getState();
    addItem({ id: items?.id, name: Date.now() }); // Assuming `items` is an array, adapt if needed.

    // Return the response data
    return response.data;
  } catch (error) {
    console.error('Error fetching items:', error);
    throw error;
  }
};

##>
usePersistanceStore.jsx

import { create } from 'zustand';
import { persist, createJSONStorage } from 'zustand/middleware';
import { mmkvStorage } from '../storage/storage'; // Using mmkvStorage for React Native
// import AsyncStorage from '@react-native-async-storage/async-storage';

// Define the store with persistence and partialize
export const usePersistanceStore = create(
  persist(
    (set, get) => ({
      // State
      fishes: 0, // Number of fishes
      logs: [], // Additional state not persisted

      // Actions
      // Add a fish to the count
      addAFish: () => set((state) => ({ fishes: state.fishes + 1 })),

      // Remove a fish from the count, ensuring it doesn't go below 0
      removeAFish: () => {
        if (get().fishes > 0) {
          set((state) => ({ fishes: state.fishes - 1 }));
        }
      },

      // Reset the fish count to 0
      resetFishes: () => set({ fishes: 0 }),

      // Add a log entry (not persisted)
      addLog: (message) => set((state) => ({ logs: [...state.logs, message] })),
    }),
    {
      // Persist configuration
      name: 'food-storage', // The key used for storage

      //optional: Persist only the `fishes` state
//  It is used to selectively persist specific keys from the state instead of storing the entire state object.
      partialize: (state) => ({ fishes: state.fishes }),

      storage: createJSONStorage(() => mmkvStorage), // // You can switch between AsyncStorage or mmkvStorage
    }
    }
  )
);

##> typscript 
import library from '@/assets/data/library.json'
import { unknownTrackImageUri } from '@/constants/images'
import { Artist, Playlist, TrackWithPlaylist } from '@/helpers/types'
import { Track } from 'react-native-track-player'
import { create } from 'zustand'

interface LibraryState {
	tracks: TrackWithPlaylist[]
	toggleTrackFavorite: (track: Track) => void
	addToPlaylist: (track: Track, playlistName: string) => void
}

export const useLibraryStore = create<LibraryState>()((set) => ({
	tracks: library,
	toggleTrackFavorite: (track) =>
		set((state) => ({
			tracks: state.tracks.map((currentTrack) => {
				if (currentTrack.url === track.url) {
					return {
						...currentTrack,
						rating: currentTrack.rating === 1 ? 0 : 1,
					}
				}

				return currentTrack
			}),
		})),
	addToPlaylist: (track, playlistName) =>
		set((state) => ({
			tracks: state.tracks.map((currentTrack) => {
				if (currentTrack.url === track.url) {
					return {
						...currentTrack,
						playlist: [...(currentTrack.playlist ?? []), playlistName],
					}
				}

				return currentTrack
			}),
		})),
}))

export const useTracks = () => useLibraryStore((state) => state.tracks)

export const useFavorites = () => {
	const favorites = useLibraryStore((state) => state.tracks.filter((track) => track.rating === 1))
	const toggleTrackFavorite = useLibraryStore((state) => state.toggleTrackFavorite)

	return {
		favorites,
		toggleTrackFavorite,
	}
}

export const useArtists = () =>
	useLibraryStore((state) => {
		return state.tracks.reduce((acc, track) => {
			const existingArtist = acc.find((artist) => artist.name === track.artist)

			if (existingArtist) {
				existingArtist.tracks.push(track)
			} else {
				acc.push({
					name: track.artist ?? 'Unknown',
					tracks: [track],
				})
			}

			return acc
		}, [] as Artist[])
	})

export const usePlaylists = () => {
	const playlists = useLibraryStore((state) => {
		return state.tracks.reduce((acc, track) => {
			track.playlist?.forEach((playlistName) => {
				const existingPlaylist = acc.find((playlist) => playlist.name === playlistName)

				if (existingPlaylist) {
					existingPlaylist.tracks.push(track)
				} else {
					acc.push({
						name: playlistName,
						tracks: [track],
						artworkPreview: track.artwork ?? unknownTrackImageUri,
					})
				}
			})

			return acc
		}, [] as Playlist[])
	})

	const addToPlaylist = useLibraryStore((state) => state.addToPlaylist)

	return { playlists, addToPlaylist }
}

##> presist
import { create } from 'zustand';
import { persist } from 'zustand/middleware';
import { getCookie, deleteCookie } from '@/utils/cookieUtil';

export type userInfo = {
  id: number;
  name: string;
};

interface AuthProps {
  user: userInfo | null; // 유저 정보
  saveUser: (user: userInfo) => void; // 유저 정보 저장
  isLogin: boolean; // 로그인 여부
  checkLogin: () => void; // 로그인 여부 확인 함수
  logout: () => void; // 로그아웃
}

export const useAuthStore = create(
  persist<AuthProps>((set) => ({
		user: null,
		isLogin: false,
		saveUser: (user) => {
		  set({ user, isLogin: true });
		},
		checkLogin: () => {
		  const accessToken = getCookie('accessToken');
		  if (accessToken) {
			set({ isLogin: true });
		  } else {
			set({ user: null, isLogin: false });
		  }
		},
		logout: () => {
		  deleteCookie('accessToken');
		  set({ user: null, isLogin: false });
		}
	}),
    {
      name: 'authStore',
      getStorage: () => {
        return localStorage;
      }
    }
  )
);