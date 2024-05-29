<script>
	import { createEventDispatcher } from 'svelte';
	import { API_URL } from '$lib/constants';
	import toast from 'svelte-french-toast';
	import { treeData, showUnoccupied } from '$lib/store';
	import questionMarkSvg from '$lib/assets/question-circle-fill.svg';

	const dispatch = createEventDispatcher();

	/**
	 * @type {string | Blob}
	 */
	let file;
	let resolution = 1;

	/**
	 * @param {{ target: { files: Blob[]; }; }} event
	 */
	function handleFileUpload(event) {
		file = event.target.files[0];
	}

	async function handleSubmit() {
		const formData = new FormData();
		formData.append('file', file);

		if (!$showUnoccupied) {
			showUnoccupied.set(true);
		}

		fetch(`${API_URL}/new-pcd?resolution=${resolution}`, {
			method: 'POST',
			body: formData
		}).then((response) => {
			if (!response.ok) {
				response.json().then((data) => {
					toast.error(data.message);
				});
			} else if (response.status === 200) {
				response.json().then((data) => {
					// Write to store
					treeData.set(data);
				});
			}
			dispatch('uploadComplete');
		});
	}
</script>

<form on:submit|preventDefault={handleSubmit} class="form-container">
	<label for="file-input" class="label">Choose a PCD file:</label>
	<input
		type="file"
		id="file-input"
		on:change={handleFileUpload}
		accept=".pcd"
		class="input"
		required
	/>

	<div class="options">
		<label for="resolution" class="label"
			>Resolution <img
				src={questionMarkSvg}
				alt="?"
				class="icon"
				title="Resolution defined in centimeter, assuming one unit in the pointcloud corresponds to one meter. The resolution determines the level of detail of the octree. Higher values result in more detailed octrees, but take more processing time."
			/> :
		</label>
		<input type="number" id="resolution" min="1" max="1000" bind:value={resolution} class="input" />
	</div>

	<button type="submit" class="button">Confirm</button>
</form>

<style>
	.form-container {
		display: flex;
		flex-direction: column;
		align-items: center;
		padding: 20px;
		max-width: 400px;
		margin: 0 auto;
	}

	.label {
		margin-bottom: 5px;
	}

	.input {
		padding: 10px;
		border: 1px solid #ccc;
		border-radius: 5px;
		margin-bottom: 10px;
		width: 100%;
		box-sizing: border-box;
	}

	.options {
		display: flex;
		flex-direction: column;
		margin-bottom: 15px;
	}

	.button {
		padding: 10px 20px;
		background-color: rgb(234, 36, 92);
		border: none;
		border-radius: 5px;
		cursor: pointer;
		transition: background-color 0.3s;
	}

	.button:hover {
		background-color: rgb(180, 31, 17);
	}
</style>
