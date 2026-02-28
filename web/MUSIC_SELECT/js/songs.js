// Song database and search functionality

let allSongs = [];
let filteredSongs = [];

// Load songs from JSON
async function loadSongs() {
  try {
    console.log('Loading songs from ./data/songs.json...');
    const response = await fetch('./data/songs.json');

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const data = await response.json();
    allSongs = data;
    filteredSongs = [...allSongs];

    console.log(`Successfully loaded ${allSongs.length} songs`);
    return allSongs;
  } catch (error) {
    console.error('Failed to load songs:', error);
    console.error('Error details:', error.message);

    // Return empty array but alert user
    alert('노래 데이터를 불러오는데 실패했습니다. 페이지를 새로고침 해주세요.');
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
