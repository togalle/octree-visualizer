<script>
	import Sidebar from '../components/Sidebar.svelte';
	import Visualizer from '../components/Visualizer.svelte';
	import toast, { Toaster } from 'svelte-french-toast';

	let showTitleCard = true;
	let titleCardClass = 'title-card';

	function hideTitleCard() {
		titleCardClass = 'title-card slide-out';
		setTimeout(() => {
			showTitleCard = false;
		}, 1000);
	}
</script>

<svelte:head>
	<title>Space partitioning</title>
	<link
		href="https://fonts.googleapis.com/css2?family=Roboto:wght@400;700&display=swap"
		rel="stylesheet"
	/>
</svelte:head>

<Toaster />
{#if showTitleCard}
	<!-- svelte-ignore a11y-click-events-have-key-events -->
	<!-- svelte-ignore a11y-no-static-element-interactions -->
	<div class={titleCardClass} on:click={hideTitleCard}>
		<h1>Octree visualizer 🐙</h1>
		<p>Click to get started!</p>
	</div>
{/if}
<div class="app-wrapper">
	<div class="sidebar"><Sidebar /></div>
	<div class="view-window">
		<Visualizer />
	</div>
</div>

<style>
	:global(html, body) {
		margin: 0;
		font-family: 'Roboto', sans-serif;
	}

	.app-wrapper {
		display: flex;
		height: 100vh;
	}

	.sidebar {
		background-color: rgb(225, 77, 51);
		display: flex;
		flex-direction: column;
		justify-content: space-between;
	}

	.view-window {
		flex: 1;
	}

	.title-card {
		position: fixed;
		top: 0;
		left: 0;
		width: 100%;
		height: 100vh;
		background-color: white;
		display: flex;
		flex-direction: column;
		justify-content: center;
		align-items: center;
	}

	.slide-out {
		animation: slideOut 1s forwards;
	}

	@keyframes slideOut {
		0% {
			transform: translateX(0);
		}
		100% {
			transform: translateX(-100%);
		}
	}
</style>
