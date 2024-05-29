<script>
	/**
	 * @type {boolean}
	 */
	export let showModal;

	/**
	 * @type {HTMLDivElement}
	 */
	let modal;

	function closeDialog() {
		showModal = false;
	}
</script>

<div class="modal-wrapper" style={showModal ? 'display: block;' : 'display: none;'}>
	<div class="modal-content">
		<div class="modal-body">
			<slot />
		</div>
	</div>
</div>

<style>
	.modal-wrapper {
		position: fixed;
		bottom: 20px;
		left: 90px;
		z-index: 9999;
	}

	.modal-content {
		background-color: #ffffff;
		border: 1px solid #ccc;
		border-radius: 5px;
		padding: 10px;
		box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
	}

	.modal-body {
		padding-top: 10px;
	}
</style>
