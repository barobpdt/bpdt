import { create } from 'zustand'

@[imports]


interface @[storeName]State {@[storeState] }

export const use@[storeName]Store = create<@[storeName]State>()((set,get) => ({@[storeImpl]})
)

/*[ex]
export const useTracks = () => useLibraryStore((state) => state.tracks)

export const useFavorites = () => {
	const favorites = useLibraryStore((state) => state.tracks.filter((track) => track.rating === 1))
	const toggleTrackFavorite = useLibraryStore((state) => state.toggleTrackFavorite)

	return {
		favorites,
		toggleTrackFavorite,
	}
}
*/
@[storeDef]