import json
import os

from matplotlib import image as mpimg
from matplotlib import pyplot as plt
from matplotlib.backend_bases import MouseEvent
from matplotlib.patches import Rectangle
from matplotlib.widgets import Button, RadioButtons

from shared import get_boundaries, get_img_preview, load_config

fig, ax = plt.subplots()
fig.subplots_adjust(left=0.3)
ax.set_xlabel("X pixel scaling")
ax.set_ylabel("Y pixels scaling")
dir_path = r'./out/png'

MULTI = 4


class Controller:
    def __init__(self):
        self.img_id = load_config()["openimg"]
        self.img_count = [os.path.isfile(os.path.join(dir_path, path)) for path in os.listdir(dir_path)].count(True)
        self.last_img = None
        self.rect_first = None
        self.rect_second = None
        self.to_set_bound_data = None
        self.text = None
        self.show_text("")

        print('Img count:', self.img_count)

    def show_text(self, text: str):
        if self.text:
            self.text.remove()
        self.text = fig.text(0.5,
                             0.05,
                             text,
                             fontsize=12,
                             bbox={
                                 "boxstyle": 'round',
                                 "facecolor": 'wheat',
                                 "alpha": 0.5
                             })
        fig.canvas.draw()

    def get_boundaries(self):
        return get_boundaries(self.img_id, MULTI)

    def set_boundaries_global(self, val: float, pl, hv, pos):
        config = load_config()
        config[pl][f"{pos}{hv}"] = int(val * MULTI)

        with open('config.json', 'w') as outfile:
            json.dump(config, outfile)
        self.draw_boundaries()

    def set_boundaries_local(self, val: float, page_id: str, pl, hv, pos):
        config = load_config()
        if config.get(page_id) is None: config[page_id] = {}
        if config[page_id].get(pl) is None: config[page_id][pl] = {}
        config[page_id][pl][f"{pos}{hv}"] = int(val * MULTI)

        with open('config.json', 'w') as outfile:
            json.dump(config, outfile)
        self.draw_boundaries()

    def set_skip(self, pl):
        def wrapped(_):
            page_id = str(self.img_id)
            config = load_config()
            if config.get(page_id) is None: config[page_id] = {}
            if config[page_id].get(pl) is None: config[page_id][pl] = {}

            skip = config[page_id][pl].get("skip", False)
            config[page_id][pl]["skip"] = not skip

            with open('config.json', 'w') as outfile:
                json.dump(config, outfile)
            self.draw_boundaries()

        return wrapped

    def draw_boundaries(self):
        skip_first, skip_second, start_x_first, end_x_first, start_y_first, end_y_first, start_x_second, end_x_second, start_y_second, end_y_second = self.get_boundaries(
        )
        if self.rect_first:
            self.rect_first.remove()
            self.rect_first = None
        if self.rect_second:
            self.rect_second.remove()
            self.rect_second = None
        if not skip_first:
            self.rect_first = ax.add_patch(
                Rectangle((start_x_first, start_y_first),
                          end_x_first - start_x_first,
                          end_y_first - start_y_first,
                          edgecolor='blue',
                          facecolor='none',
                          lw=1))
        if not skip_second:
            self.rect_second = ax.add_patch(
                Rectangle((start_x_second, start_y_second),
                          end_x_second - start_x_second,
                          end_y_second - start_y_second,
                          edgecolor='red',
                          facecolor='none',
                          lw=1))
        fig.canvas.draw()

    def load_img(self, id: str | int):
        image = mpimg.imread(get_img_preview(int(id)))
        print("IMAGE:", image)
        if image is None:
            return
        self.img_id = int(id)
        ax.set_title(get_img_preview(self.img_id))
        if self.last_img:
            self.last_img.remove()
        self.last_img = ax.imshow(image)

        self.draw_boundaries()

    def onclick(self, event: MouseEvent):
        print(ax == event.inaxes)
        if ax == event.inaxes and self.to_set_bound_data is not None:
            val = event.xdata if self.to_set_bound_data["hv"] == "x" else event.ydata
            if self.to_set_bound_data["ty"] == "Global":
                self.set_boundaries_global(val, self.to_set_bound_data["pl"], self.to_set_bound_data["hv"],
                                           self.to_set_bound_data["pos"])
            else:
                self.set_boundaries_local(val, str(self.img_id), self.to_set_bound_data["pl"],
                                          self.to_set_bound_data["hv"], self.to_set_bound_data["pos"])
            self.to_set_bound_data = None
            self.show_text("")
        # print('%s click: button=%d, x=%d, y=%d, xdata=%f, ydata=%f' %
        #       ('double' if event.dblclick else 'single', event.button, event.x, event.y, event.xdata, event.ydata))

    def button_bound(self, ty, pl, hv, pos):
        def wrapped(_):
            self.to_set_bound_data = {"ty": ty, "pl": pl, "hv": hv, "pos": pos}
            self.show_text(f"{ty} {pl} {hv} {pos}")
            print(self.to_set_bound_data)

        return wrapped

    def next(self, _):
        self.load_img(self.img_id + 1)

    def prev(self, _):
        self.load_img(self.img_id - 1)


controller = Controller()
fig.canvas.mpl_connect('button_press_event', controller.onclick)
controller.load_img(controller.img_id)

axcolor = 'lightgoldenrodyellow'
rax = fig.add_axes([0.05, 0.05, 0.10, 0.80], facecolor=axcolor)
radio = RadioButtons(rax, [str(x) for x in range(0, controller.img_count, 5)])


def radio_click(label):
    controller.load_img(label)


radio.on_clicked(radio_click)

buttons = []


def add_button(title: str, ax: list[float], callback):
    axf = fig.add_axes(ax)
    button = Button(axf, title)
    button.on_clicked(callback)
    buttons.append(button)


add_button('Previous', [0.7, 0.05, 0.1, 0.075], controller.prev)
add_button('Next', [0.81, 0.05, 0.1, 0.075], controller.next)
add_button('Swap skip first', [0.25, 0.05, 0.07, 0.075], controller.set_skip("first"))
add_button('Swap skip second', [0.35, 0.05, 0.07, 0.075], controller.set_skip("second"))


def get_bound_button_coords(col: int, row: int) -> list[float]:
    return [0.05 + 0.11 * col, 0.95 - row * 0.05, 0.1, 0.04]


for i, ty in enumerate(["Global", "Local"]):
    for j, pl in enumerate(["first", "second"]):
        for k, hv in enumerate(["x", "y"]):
            for l, pos in enumerate(["start", "end"]):
                add_button(f'{ty} {pl} {hv} {pos}', get_bound_button_coords(j * 4 + k * 2 + l, i),
                           controller.button_bound(ty, pl, hv, pos))

plt.show()