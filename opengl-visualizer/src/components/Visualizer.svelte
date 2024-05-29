<script>
	// @ts-nocheck

	import * as THREE from 'three';
	import { onMount } from 'svelte';
	import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
	import { renderLight, showUnoccupied, treeData, treeDepth } from '$lib/store';
	import { WHITE } from '$lib/constants';
	import { BufferGeometryUtils } from 'three/examples/jsm/Addons.js';
	import { vertexColor } from 'three/examples/jsm/nodes/Nodes.js';

	let camera;
	let data;
	let initialized = false;
	treeData.subscribe((value) => {
		data = value;
		if (data) treeDepth.set(data.treeDepth);
		if (!initialized) main();
	});
	let mergedOccupiedGeometries = null;
	let mergedUnoccupiedGeometries = null;

	let showUnoccupiedValue = true;
	showUnoccupied.subscribe((value) => {
		showUnoccupiedValue = value;
	});

	function main() {
		if (!data) {
			return;
		} else {
			initialized = true;
		}
		const canvas = document.querySelector('#c');
		const renderer = new THREE.WebGLRenderer({ antialias: true, canvas });
		renderer.setClearColor(WHITE);

		// Setup settings
		const fov = 75;
		const aspect = 2;
		const near = 0.1;
		const far = data.rootSize * 10;
		if (!camera) {
			camera = new THREE.PerspectiveCamera(fov, aspect, near, far);
			camera.position.set(data.rootSize, data.rootSize, data.rootSize);
		}
		const controls = new OrbitControls(camera, renderer.domElement);
		controls.target.set(0, 0, 0);
		controls.listenToKeyEvents(window);
		controls.screenSpacePanning = true;
		controls.update();
		const scene = new THREE.Scene();

		scene.add(camera);

		// Ambient light and light relative to camera for easy viewing
		const ambientLight = new THREE.AmbientLight(0xffffff, 1);
		scene.add(ambientLight);
		const light = new THREE.DirectionalLight(0xffffff, 3);
		light.position.set(1, 1, 1).normalize();
		camera.add(light);

		const color = new THREE.Color();

		let unoccupiedMesh;
		let occupiedMesh;
		function addMeshes(useLight = false, updateGeometry = false) {
			if (updateGeometry) {
				const unoccupiedGeometries = [];
				const occupiedGeometries = [];
				let voxelSizes = [data.rootSize];
				for (let i = 1; i <= data.treeDepth; i++) {
					voxelSizes.push(data.rootSize / Math.pow(2, i));
				}
				let boxGeometry = new THREE.BoxGeometry(1, 1, 1);

				const red = new THREE.Color(0xff0000);
				const green = new THREE.Color(0x00ff00);
				const blue = new THREE.Color(0x0000ff);

				const minHeight = -data.rootSize / 2;
				const maxHeight = data.rootSize / 2;

				// Create voxels
				for (const voxel of data.voxels) {
					if (!voxel.o && !showUnoccupiedValue) continue;

					// Box geometry
					const voxelSize = voxelSizes[voxel.d];
					let voxelGeometry = boxGeometry.clone();
					voxelGeometry.scale(voxelSize, voxelSize, voxelSize);

					if (voxel.o) {
						// Height color
						const normalizedHeight = (voxel.c.y - minHeight) / (maxHeight - minHeight);
						if (normalizedHeight < 0.5) {
							color.lerpColors(blue, green, normalizedHeight * 2);
						} else {
							color.lerpColors(green, red, (normalizedHeight - 0.5) * 2);
						}
						const rgb = color.toArray().map((v) => v * 255);
						// make an array to store colors for each vertex
						const numVerts = boxGeometry.getAttribute('position').count;
						const itemSize = 3; // r, g, b
						const colors = new Uint8Array(itemSize * numVerts);

						// copy the color into the colors array for each vertex
						colors.forEach((v, ndx) => {
							colors[ndx] = rgb[ndx % 3];
						});

						const colorAttrib = new THREE.BufferAttribute(colors, itemSize, true);
						voxelGeometry.setAttribute('color', colorAttrib);
					}

					// Set geometry location
					voxelGeometry.translate(voxel.c.x, voxel.c.y, voxel.c.z);

					if (voxel.o) {
						occupiedGeometries.push(voxelGeometry);
					} else {
						unoccupiedGeometries.push(voxelGeometry);
					}
				}

				mergedUnoccupiedGeometries = BufferGeometryUtils.mergeGeometries(
					unoccupiedGeometries,
					false
				);
				mergedOccupiedGeometries = BufferGeometryUtils.mergeGeometries(occupiedGeometries, false);
			}

			let occupiedMaterial;
			if (useLight) {
				occupiedMaterial = new THREE.MeshLambertMaterial({ vertexColors: true });
			} else {
				occupiedMaterial = new THREE.MeshBasicMaterial({ vertexColors: true });
			}
			const unoccupiedMaterial = new THREE.LineBasicMaterial({ color: 0x000000 });

			unoccupiedMesh = new THREE.LineSegments(
				new THREE.EdgesGeometry(mergedUnoccupiedGeometries),
				unoccupiedMaterial
			);
			occupiedMesh = new THREE.Mesh(mergedOccupiedGeometries, occupiedMaterial);

			if (showUnoccupiedValue) scene.add(unoccupiedMesh);
			scene.add(occupiedMesh);
		}

		addMeshes(false, true);

		function resizeRendererToDisplaySize(renderer) {
			const canvas = renderer.domElement;
			const width = canvas.clientWidth;
			const height = canvas.clientHeight;
			const needResize = canvas.width !== width || canvas.height !== height;
			if (needResize) {
				renderer.setSize(width, height, false);
			}

			return needResize;
		}

		function render() {
			// Responsiveness
			if (resizeRendererToDisplaySize(renderer)) {
				console.log("Resizing renderer's display size");
				const canvas = renderer.domElement;
				camera.aspect = canvas.clientWidth / canvas.clientHeight;
				camera.updateProjectionMatrix();
			}

			renderer.render(scene, camera);
		}

		// Toggle unoccupied voxels
		showUnoccupied.subscribe((value) => {
			if (value && !unoccupiedMesh.visible) {
				unoccupiedMesh.visible = true;
			} else {
				unoccupiedMesh.visible = false;
			}
			render();
		});

		renderLight.subscribe((value) => {
			if (value) {
				occupiedMesh.material = new THREE.MeshLambertMaterial({ vertexColors: true });
			} else {
				occupiedMesh.material = new THREE.MeshBasicMaterial({ vertexColors: true });
			}
			render();
		});

		// Update data
		treeData.subscribe((value) => {
			if (data) {
				// console.log('Rendering scene');
				scene.remove(unoccupiedMesh);
				scene.remove(occupiedMesh);
				// Dispose of old geometries
				unoccupiedMesh.geometry.dispose();
				occupiedMesh.geometry.dispose();
				// Dispose of old materials
				unoccupiedMesh.material.dispose();
				occupiedMesh.material.dispose();
				addMeshes($renderLight, true);
				render();
			}
		});

		render();

		controls.addEventListener('change', render);
	}

	onMount(() => {
		main();
	});
</script>

{#if !data}
	<div class="tips">
		<p class="tip">Use the plus button to <b>create</b> a new octree 🐙</p>
		<p class="tip">
			Use the settings button to change how the current octree is <b>rendered</b> 🔧
		</p>
	</div>
{/if}

<canvas id="c" />

<style>
	#c {
		width: 100%;
		height: 100%;
		display: block;
	}

	.tips {
		display: flex;
		flex-direction: column;
		justify-content: space-between;
		height: 100%;
	}
	.tip {
		margin: 25px 0 25px 10px;
	}
</style>
