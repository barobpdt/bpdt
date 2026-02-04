// Song database and search functionality

let allSongs = [];
let filteredSongs = [];

// Load songs from JSON
async function loadSongs() {
  try {
    const response = await fetch('./data/songs.json');
    allSongs = await response.json();
    filteredSongs = [...allSongs];
    return allSongs;
  } catch (error) {
    console.error('Failed to load songs:', error);
    return [];
  }
}

// Search songs by query (title, artist, or number)
function searchSongs(query) {
  if (!query || query.trim() === '') {
    filteredSongs = [...allSongs];
    return filteredSongs;
  }
  
  const searchTerm = query.toLowerCase().trim();
  
  filteredSongs = allSongs.filter(song => {
    return (
      song.title.toLowerCase().includes(searchTerm) ||
      song.artist.toLowerCase().includes(searchTerm) ||
      song.number.includes(searchTerm)
    );
  });
  
  return filteredSongs;
}

// Filter songs by category
function filterByCategory(category) {
  if (!category || category === 'all') {
    filteredSongs = [...allSongs];
    return filteredSongs;
  }
  
  filteredSongs = allSongs.filter(song => song.category === category);
  return filteredSongs;
}

// Filter songs by language
function filterByLanguage(language) {
  if (!language || language === 'all') {
    filteredSongs = [...allSongs];
    return filteredSongs;
  }
  
  filteredSongs = allSongs.filter(song => song.language === language);
  return filteredSongs;
}

// Combined filter (category + search)
function applyFilters(category, searchQuery) {
  let results = [...allSongs];
  
  // Apply category filter
  if (category && category !== 'all') {
    results = results.filter(song => song.category === category);
  }
  
  // Apply search filter
  if (searchQuery && searchQuery.trim() !== '') {
    const searchTerm = searchQuery.toLowerCase().trim();
    results = results.filter(song => {
      return (
        song.title.toLowerCase().includes(searchTerm) ||
        song.artist.toLowerCase().includes(searchTerm) ||
        song.number.includes(searchTerm)
      );
    });
  }
  
  filteredSongs = results;
  return filteredSongs;
}

// Get all unique categories
function getCategories() {
  const categories = new Set(allSongs.map(song => song.category));
  return ['all', ...Array.from(categories)];
}

// Get song by ID
function getSongById(id) {
  return allSongs.find(song => song.id === id);
}

// Export functions
window.SongManager = {
  loadSongs,
  searchSongs,
  filterByCategory,
  filterByLanguage,
  applyFilters,
  getCategories,
  getSongById,
  getAllSongs: () => allSongs,
  getFilteredSongs: () => filteredSongs
};
