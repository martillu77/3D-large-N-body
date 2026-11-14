import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button


class Renderer:
    def __init__(self, snapshots):
        self.snapshots = snapshots
        self.n_snapshots = len(snapshots)
        self.current_index = 0

        # Playback settings
        self.playing = False
        self.fps = 20
        self.frame_step = 1

        self.fig = plt.figure(figsize=(11, 8))
        self.ax = self.fig.add_axes([0.08, 0.25, 0.84, 0.68],
                                    projection="3d")

        # Initial view: use only the first snapshot
        initial_positions = snapshots[0].positions

        lower = np.min(initial_positions, axis=0)
        upper = np.max(initial_positions, axis=0)
        center = 0.5 * (lower + upper)

        half_range = 0.55 * np.max(upper - lower)
        if half_range == 0:
            half_range = 1.0

        self.ax.set_xlim(center[0] - half_range,
                         center[0] + half_range)
        self.ax.set_ylim(center[1] - half_range,
                         center[1] + half_range)
        self.ax.set_zlim(center[2] - half_range,
                         center[2] + half_range)

        self.initial_limits = (
            self.ax.get_xlim(),
            self.ax.get_ylim(),
            self.ax.get_zlim(),
        )

        self.ax.set_xlabel("X")
        self.ax.set_ylabel("Y")
        self.ax.set_zlabel("Z")

        self.scatter = self.ax.scatter([], [], [], s=8)

        # Snapshot slider
        slider_ax = self.fig.add_axes([0.15, 0.17, 0.7, 0.025])
        self.slider = Slider(
            slider_ax,
            "Snapshot",
            0,
            max(1, self.n_snapshots - 1),
            valinit=0,
            valstep=1,
        )
        self.slider.on_changed(self.on_slider)

        # Playback buttons
        play_ax = self.fig.add_axes([0.35, 0.08, 0.10, 0.05])
        pause_ax = self.fig.add_axes([0.47, 0.08, 0.10, 0.05])

        self.play_button = Button(play_ax, "Play")
        self.pause_button = Button(pause_ax, "Pause")

        self.play_button.on_clicked(self.play)
        self.pause_button.on_clicked(self.pause)

        # FPS slider
        fps_ax = self.fig.add_axes([0.65, 0.13, 0.22, 0.025])
        self.fps_slider = Slider(
            fps_ax,
            "FPS",
            1,
            60,
            valinit=self.fps,
            valstep=1,
        )
        self.fps_slider.on_changed(self.set_fps)

        # Frame step slider
        step_ax = self.fig.add_axes([0.65, 0.08, 0.22, 0.025])
        self.step_slider = Slider(
            step_ax,
            "Frame step",
            1,
            100,
            valinit=self.frame_step,
            valstep=1,
        )
        self.step_slider.on_changed(self.set_frame_step)

        # Timer
        self.timer = self.fig.canvas.new_timer(
            interval=1000 // self.fps
        )
        self.timer.add_callback(self.advance)

        # Mouse and keyboard controls
        self.fig.canvas.mpl_connect("scroll_event", self.on_scroll)
        self.fig.canvas.mpl_connect("key_press_event", self.on_key)

        self.update(0)

    def update(self, index):
        index = int(index)
        self.current_index = index

        snapshot = self.snapshots[index]
        positions = snapshot.positions

        self.scatter._offsets3d = (
            positions[:, 0],
            positions[:, 1],
            positions[:, 2],
        )

        self.ax.set_title(
            f"Step: {snapshot.step}    "
            f"Time: {snapshot.time:.6g}    "
            f"Frame: {index + 1}/{self.n_snapshots}"
        )

        self.fig.canvas.draw_idle()


    def set_frame_step(self, value):
        self.frame_step = int(value)

    def on_slider(self, value):
        self.update(int(value))

    def play(self, event=None):
        if self.current_index >= self.n_snapshots - 1:
            self.slider.set_val(0)

        self.playing = True
        self.timer.start()

    def pause(self, event=None):
        self.playing = False
        self.timer.stop()

    def advance(self):
        if not self.playing:
            return

        next_index = self.current_index + self.frame_step

        if next_index >= self.n_snapshots - 1:
            next_index = self.n_snapshots - 1
            self.slider.set_val(next_index)
            self.pause()
            return

        self.slider.set_val(next_index)

    def set_fps(self, value):
        self.fps = int(value)
        self.timer.stop()
        self.timer.interval = max(1, round(1000 / self.fps))

        if self.playing:
            self.timer.start()

    def on_scroll(self, event):
        if event.inaxes != self.ax:
            return

        factor = 0.8 if event.button == "up" else 1.25

        xlim = self.ax.get_xlim()
        ylim = self.ax.get_ylim()
        zlim = self.ax.get_zlim()

        x_mouse = event.xdata
        y_mouse = event.ydata

        def zoom(limits, mouse):
            lower, upper = limits
            return (
                mouse + (lower - mouse) * factor,
                mouse + (upper - mouse) * factor,
            )

        self.ax.set_xlim(zoom(xlim, x_mouse))
        self.ax.set_ylim(zoom(ylim, y_mouse))

        z_center = 0.5 * (zlim[0] + zlim[1])
        z_half = 0.5 * (zlim[1] - zlim[0]) * factor
        self.ax.set_zlim(
            z_center - z_half,
            z_center + z_half,
        )

        self.fig.canvas.draw_idle()

    def reset_zoom(self):
        self.ax.set_xlim(self.initial_limits[0])
        self.ax.set_ylim(self.initial_limits[1])
        self.ax.set_zlim(self.initial_limits[2])
        self.fig.canvas.draw_idle()

    def on_key(self, event):
        index = self.current_index

        if event.key == "right":
            self.pause()
            index = min(index + 1, self.n_snapshots - 1)

        elif event.key == "left":
            self.pause()
            index = max(index - 1, 0)

        elif event.key == "pageup":
            self.pause()
            index = min(index + 100, self.n_snapshots - 1)

        elif event.key == "pagedown":
            self.pause()
            index = max(index - 100, 0)

        elif event.key == " ":
            if self.playing:
                self.pause()
            else:
                self.play()
            return

        elif event.key == "r":
            self.reset_zoom()
            return

        elif event.key == "q":
            plt.close(self.fig)
            return

        else:
            return

        self.slider.set_val(index)

    def show(self):
        plt.show()