##> config	
	isFolderCreate = true
	projectPath = @[path]/project/test
	basePath = @[path]/project/test
	cmd {
	
	}
	set {
		useTypescript = conf('baro.useTypescript')
		version = apiCall( conf('baro.apiUrl') )
	}

##> config {name=backend}
	projectName = backend
	projectPath = @[config.projectPath]/project/test
	folders=[config, controllers, routes, logs]

##> config {name=frontend}
	projectName = frontend
	projectPath = @[basePath]/project/frontend

##> store
auto{
	a &TrackWithPlaylist[] | null	// 테스트
	b	// test1
	c
	d=0
	e=[]
	log=[] &LogItem[]
	items=[]
	ff={id,name,value} &UserInfo	// test2
	seta(a)				// test 3
	setb(b) {b:aaa}			// test4
	async fetchItem() {
		const response = await axios.get(`${BASE_URL}/items`); // Fetching items
		const data = response.data;
		set({ items: data });
	}
}
state {
	fishes: 0, // Number of fishes
	logs: [], // Additional state not persisted
	addAFish: () => set((state) => ({ fishes: state.fishes + 1 })),
}
persist {
	log:[],
	item
}
template {
	import { create } from 'zustand';
	import { persist } from 'zustand/middleware';
	interface AuthProps { @[interface] }
	export const useAuthStore = create(
	  persist<AuthProps>((set) => ({@[persist]}), {@[state]})
	);
}

##> store {name=test}
auto{
	a=0
	b=0
	c
}