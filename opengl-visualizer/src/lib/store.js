import { writable } from 'svelte/store';

export const darkMode = writable(false);
export const treeData = writable(null);
export const showUnoccupied = writable(true);
export const treeDepth = writable(1);
export const renderLight = writable(false);