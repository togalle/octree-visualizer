<script>
	import { DARK_GRAY, WHITE } from '$lib/constants';
	import { darkMode } from '$lib/store';

	/**
	 * @type {boolean}
	 */
	export let showModal;

	/**
	 * @type {HTMLDialogElement}
	 */
	let dialog;

	$: if (dialog && showModal) dialog.showModal();
</script>

<!-- svelte-ignore a11y-click-events-have-key-events a11y-no-noninteractive-element-interactions -->
<dialog
	bind:this={dialog}
	on:close={() => (showModal = false)}
	on:click|self={() => dialog.close()}
>
	<!-- svelte-ignore a11y-no-static-element-interactions -->
	<div on:click|stopPropagation>
		<div class="modal-header">
			<slot name="header" /> <button class="close-button" on:click={() => dialog.close()}>X</button>
		</div>
		<div class="modal-body">
			<slot />
		</div>
		<!-- svelte-ignore a11y-autofocus -->
	</div>
</dialog>

<style>
	dialog {
		max-width: 32em;
		border-radius: 0.5em;
		border: none;
		padding: 0;
	}
	dialog::backdrop {
		background: rgba(0, 0, 0, 0.3);
		transition: background 1s ease;
	}
	.modal-header {
		padding: 0 1em 0 1em;
	}
	.modal-body {
		padding: 1em;
	}
	dialog[open] {
		animation: zoom 0.3s cubic-bezier(0.34, 1.56, 0.64, 1);
	}
	@keyframes zoom {
		from {
			transform: scale(0.95);
		}
		to {
			transform: scale(1);
		}
	}
	dialog[open]::backdrop {
		animation: fade 0.2s ease-out;
	}
	@keyframes fade {
		from {
			opacity: 0;
		}
		to {
			opacity: 1;
		}
	}
	button {
		display: block;
	}
	.close-button {
		background: none;
		border: none;
		font-size: 1.5em;
		cursor: pointer;
	}

	.modal-header {
		display: flex;
		justify-content: space-between;
		background-color: rgb(225, 77, 51);
	}
</style>
