import tkinter as tk
from tkinter import filedialog, messagebox, ttk
from PIL import Image, ImageTk
from creat_circular import *

class CircularApp:
    def __init__(self, master):
        self.master = master
        self.master.title("GIF轉換成圓盤")
        self.master.geometry("400x500")
        self.master.resizable(False, False)      
        self.gif_path = None
        self.circular_image = None
        
        button_frame = tk.Frame(self.master)
        button_frame.pack(pady=10)
        
        # Combobox選擇紙張大小
        self.paper_size_var = tk.StringVar()
        self.paper_size_var.set("A4")
        self.paper_size_combobox = ttk.Combobox(button_frame, textvariable=self.paper_size_var, values=["A4", "A5"], width=5)
        self.paper_size_combobox.pack(side=tk.LEFT, padx=5)

        self.sorce_button = tk.Button(button_frame, text="選擇來源", command=self.image_gif_switch)
        self.sorce_button.pack(side=tk.LEFT, padx=5)

        self.scale_var = tk.DoubleVar(value=1.0)
        self.height_var = tk.DoubleVar(value=1.0)
        
        self.refresh_button = tk.Button(button_frame, text="刷新", command=self.refresh_image)
        self.refresh_button.pack(side=tk.LEFT, padx=5)

        self.save_button = tk.Button(button_frame, text="保存", command=self.save_image, state=tk.DISABLED)
        self.save_button.pack(side=tk.LEFT, padx=5)

        self.flip_var = tk.BooleanVar()
        self.flip_check = tk.Checkbutton(button_frame, text="翻轉", variable=self.flip_var, command=self.refresh_image)
        self.flip_check.pack(side=tk.LEFT, padx=5)

        self.mode_var = tk.BooleanVar()
        self.mode_check = tk.Checkbutton(button_frame, text="單張", variable=self.mode_var)
        self.mode_check.pack(side=tk.LEFT, padx=5)

        slider_frame = tk.Frame(self.master)
        slider_frame.pack(pady=5)
        
        self.scale_slider = tk.Scale(slider_frame, from_=0, to=3.0, resolution=0.05,
                                     orient=tk.HORIZONTAL, label="縮放比例", variable=self.scale_var,
                                     length=150)
        self.scale_slider.pack(side=tk.LEFT, padx=5)
        
        self.height_slider = tk.Scale(slider_frame, from_=0, to=1.5, resolution=0.05,
                                      orient=tk.HORIZONTAL, label="高度比例", variable=self.height_var,
                                      length=150)
        self.height_slider.pack(side=tk.RIGHT, padx=5)


        
        self.canvas = tk.Canvas(self.master, width=500, height=500)
        self.canvas.pack(pady=10)

    def image_gif_switch(self):
        if self.mode_var.get():
            self.choose_image()
        else:
            self.choose_gif()
        return

    def choose_image(self):
        self.image_path = filedialog.askopenfilename(title="選擇圖片文件", filetypes=[("Image files", "*.png *.jpg *.jpeg *.bmp *.tiff")])
        if not self.image_path:
            messagebox.showerror("錯誤", "未選擇文件")
            return
        self.circular_image = create_circular_single_image(self.image_path, 6, 300, (255, 255, 255))
        self.save_button.config(state=tk.NORMAL)
        circular_image2 = self.circular_image.resize((350, 350), Image.LANCZOS)
        self.show_image(circular_image2) 

    def choose_gif(self):
        self.gif_path = filedialog.askopenfilename(title="選擇GIF文件", filetypes=[("GIF files", "*.gif")])
        if not self.gif_path:
            messagebox.showerror("錯誤", "未選擇文件")
            return
        
        self.circular_image = create_circular_animation(self.gif_path, radius_cm=6, dpi=300, background_color=(255, 255, 255))
        self.save_button.config(state=tk.NORMAL)
        #resize image to fit canvas and just for show ,don't change the original image
        circular_image2 = self.circular_image.resize((350, 350), Image.LANCZOS)
        self.show_image(circular_image2)

    def refresh_image(self):
        if not (self.gif_path or self.image_path):
            messagebox.showerror("錯誤", "未選擇文件")
            return
        if self.mode_var.get():
            self.circular_image = create_circular_single_image(self.image_path, 6, 300, (255, 255, 255),self.scale_var.get(),self.height_var.get(),self.flip_var.get())
        else:
            self.circular_image = create_circular_animation(self.gif_path, 6,300,(255, 255, 255),self.scale_var.get(),self.height_var.get(),self.flip_var.get())
        circular_image2 = self.circular_image.resize((350, 350), Image.LANCZOS)
        self.show_image(circular_image2)

    def show_image(self,show_img):
        self.image = ImageTk.PhotoImage(show_img)
        self.canvas.create_image(200, 180, anchor=tk.CENTER, image=self.image)

    def save_image(self):
        dpi=300
        page_image=combine2page(self.circular_image, dpi=dpi, page_size=self.paper_size_var.get())
        #將此圖片轉換成pdf並且保存
        pdf_output_path = filedialog.asksaveasfilename(defaultextension=".pdf", filetypes=[("PDF files", "*.pdf")], title="保存PDF")
        if not pdf_output_path:
            messagebox.showerror("錯誤", "未選擇保存位置")
            return
        page_image.save(pdf_output_path, 'PDF', dpi=(dpi, dpi))
        messagebox.showinfo("成功", "保存成功")

if __name__ == "__main__":
    root = tk.Tk()
    app = CircularApp(root)
    root.mainloop()
