<script>
	import { treeData, showUnoccupied, treeDepth, renderLight } from '$lib/store';
	import Modal from './Modal.svelte';
	import NewTree from './NewTree.svelte';
	import { API_URL } from '$lib/constants';
	import SettingsModal from './SettingsModal.svelte';

	export let showNew = false;
	function closeNewModal() {
		showNew = false;
	}
	let showSettings = false;
	let treeDepthSlider = 1;

	treeDepth.subscribe((value) => {
		treeDepthSlider = value;
	});

	function updateTreeDepth() {
		if (!$showUnoccupied) {
			showUnoccupied.set(true);
		}
		fetch(API_URL + `/octree?height=${treeDepthSlider}`)
			.then((response) => response.json())
			.then((data) => {
				if (data != null) {
					treeData.set(data);
				}
			})
			.catch((error) => console.error('Error:', error));
	}
</script>

<button
	on:click={() => (showNew = true)}
	aria-label="New pointcloud"
	id="upload-button"
	class="button"
	title="New pointcloud"
/>
<Modal bind:showModal={showNew}>
	<h3 slot="header">New tree</h3>
	<NewTree on:uploadComplete={closeNewModal} />
</Modal>

<button
	on:click={() => (showSettings = !showSettings)}
	aria-label="Settings"
	id="settings-button"
	class="button"
	title="Settings"
/>
<SettingsModal bind:showModal={showSettings}>
	<div class="setting">
		<label for="treeDepth">Octree Depth:</label>
		<div class="input-group">
			<input id="treeDepth" type="range" bind:value={treeDepthSlider} min="1" max="15" />
			<span class="treedepth-display">{treeDepthSlider}</span>
			<button on:click={updateTreeDepth}>Apply</button>
		</div>
	</div>
	<div class="setting">
		<label for="showUnoccupied">Render Wireframe Cubes:</label>
		<input id="showUnoccupied" type="checkbox" bind:checked={$showUnoccupied} />
	</div>
	<div class="setting">
		<label for="renderLight">Render Light:</label>
		<input id="renderLight" type="checkbox" bind:checked={$renderLight} />
	</div>
</SettingsModal>

<style>
	#upload-button {
		background: url('../lib/assets/plus-circle-fill.svg') no-repeat;
		background-size: contain;
	}

	#settings-button {
		background: url('../lib/assets/gear-fill.svg') no-repeat;
		background-size: contain;
	}

	.button {
		border: none;
		width: 3em;
		height: 3em;
		margin: 1em;
		cursor: pointer;
		transition: filter 0.2s ease;
	}

	.button:hover {
		filter: invert(100%);
	}

	.setting {
		margin-bottom: 1em;
	}
	.input-group {
		display: flex;
		align-items: center;
		gap: 10px;
	}
	.treedepth-display {
		width: 2ch;
		text-align: center;
	}
	label {
		display: block;
		margin-bottom: 0.5em;
	}
</style>
