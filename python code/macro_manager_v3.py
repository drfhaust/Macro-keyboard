import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import json
import webbrowser

class MacroManagerApp:
    def __init__(self, master):
        self.master = master
        self.center_window(900, 600)  # Set default window size and center it

        master.title("Macro Manager")
        master.configure(bg="SystemButtonFace")
        master.resizable(True, True)

        # Initialize data structures
        self.macros = {}
        self.buttons = {}
        self.active_button = None
        self.save_directory = None
        self.active_modifier = None
        self.modifier_buttons = {}

        # Create UI components
        self.create_widgets()

    def center_window(self, width, height):
        """Center the window on the screen."""
        screen_width = self.master.winfo_screenwidth()
        screen_height = self.master.winfo_screenheight()
        x_coordinate = (screen_width // 2) - (width // 2)
        y_coordinate = (screen_height // 2) - (height // 2)
        self.master.geometry(f"{width}x{height}+{x_coordinate}+{y_coordinate}")

    def create_widgets(self):
        padding_options = (10, 5)

        # Button frame for GUI layout with keypad background color set
        button_frame = tk.Frame(self.master, bg="#03194b", highlightthickness=0)
        button_frame.pack(side="left", padx=20, pady=20)

        # Create a "Home" button separately
        self.home_button = tk.Button(button_frame, text="Home", width=5, height=2, bg="SystemButtonFace", relief="raised")
        self.home_button.grid(row=0, column=0, padx=10, pady=10)

        # Generate buttons 1 to 12 and place them in a 4x3 grid (for 12 buttons)
        for i in range(1, 13):
            btn = tk.Button(button_frame, text=str(i), width=5, height=2, command=lambda b=i: self.select_button(b), bg="SystemButtonFace", relief="raised")
            row, col = divmod(i-1, 4)
            btn.grid(row=row+1, column=col, padx=10, pady=10)
            self.buttons[f"button{i}"] = btn

        # Modifier buttons frame (with reduced padding)
        modifier_frame = ttk.LabelFrame(self.master, text="Select Action", padding=(5, 5))  # Reduced padding
        modifier_frame.pack(fill="x", padx=10, pady=5)

        # Modifier buttons
        modifier_buttons_frame = tk.Frame(modifier_frame)
        modifier_buttons_frame.pack(padx=5, pady=5, anchor="center")  # Reduced padding

        modifiers = ["open", "copy", "paste", "screenshot"]
        for mod in modifiers:
            btn = tk.Button(modifier_buttons_frame, text=mod.capitalize(), width=10, command=lambda m=mod: self.set_modifier(m))
            btn.pack(side="left", padx=5, pady=5)  # Reduced padding
            self.modifier_buttons[mod] = btn

        # Config section (without Select Button dropdown)
        config_frame = ttk.LabelFrame(self.master, text="Configure Button", padding=padding_options)
        config_frame.pack(fill="x", padx=10, pady=5)

        # Entry for value and button text
        ttk.Label(config_frame, text="Macro Value (Path or Status):").grid(row=0, column=0, padx=10, pady=5)
        self.value_entry = ttk.Entry(config_frame, width=40)
        self.value_entry.grid(row=0, column=1, padx=10, pady=5)

        # Folder/File Select Buttons
        self.browse_button = ttk.Button(config_frame, text="Browse", command=self.select_file_folder)
        self.browse_button.grid(row=0, column=2, padx=10, pady=5, sticky="ew")

        ttk.Label(config_frame, text="Button Text:").grid(row=1, column=0, padx=10, pady=5)
        self.text_entry = ttk.Entry(config_frame, width=40)
        self.text_entry.grid(row=1, column=1, padx=10, pady=5)

        # Add/Update Button
        self.add_button = ttk.Button(config_frame, text="Add/Update Macro", command=self.add_macro)
        self.add_button.grid(row=2, column=1, padx=10, pady=5)

        # Treeview for showing the assigned macros
        macros_frame = ttk.LabelFrame(self.master, text="Assigned Macros", padding=padding_options)
        macros_frame.pack(fill="both", expand=True, padx=10, pady=5)

        columns = ("Button", "Action", "Value", "Text")
        self.tree = ttk.Treeview(macros_frame, columns=columns, show="headings")
        for col in columns:
            self.tree.heading(col, text=col)
            self.tree.column(col, width=100 if col == "Button" else 150)
        self.tree.pack(fill="both", expand=True)

        # Double-click event for the Treeview
        self.tree.bind("<Double-1>", self.on_treeview_double_click)

        # Load and Save buttons
        control_frame = ttk.Frame(self.master)
        control_frame.pack(fill="x", padx=10, pady=5)

        self.load_button = ttk.Button(control_frame, text="Load JSON", command=self.load_json)
        self.load_button.pack(side="left", padx=10)

        self.save_button = ttk.Button(control_frame, text="Save JSON", command=self.save_json)
        self.save_button.pack(side="left", padx=10)

        self.select_directory_button = ttk.Button(control_frame, text="Select Save Directory", command=self.select_save_directory)
        self.select_directory_button.pack(side="left", padx=10)

        # Credits section (new container below the keypad)
        credits_frame = tk.Frame(self.master, bg="black")
        credits_frame.pack(side="bottom", padx=10, pady=10, fill="x", anchor="s")  # Separate container below keypad
        
        credits_label = tk.Label(credits_frame, text="Developed by Daniel Olaifa", fg="white", bg="black")
        credits_label.pack(side="left", padx=10)
        
        github_link = tk.Label(credits_frame, text="Visit my GitHub", fg="blue", bg="black", cursor="hand2")
        github_link.pack(side="left", padx=10)
        github_link.bind("<Button-1>", lambda e: webbrowser.open_new_tab('https://github.com/drfhaust'))
        
        portfolio_link = tk.Label(credits_frame, text="Visit my Portfolio", fg="blue", bg="black", cursor="hand2")
        portfolio_link.pack(side="left", padx=10)
        portfolio_link.bind("<Button-1>", lambda e: webbrowser.open_new_tab('https://www.oluwadara.tech/aboutme'))

    def set_modifier(self, modifier):
        """Set the active modifier and update the UI."""
        self.active_modifier = modifier
        # Update the buttons to reflect the current active modifier
        for mod, btn in self.modifier_buttons.items():
            if mod == modifier:
                btn.config(relief="sunken", bg="lightblue")
            else:
                btn.config(relief="raised", bg="SystemButtonFace")

        if modifier in ["open", "paste"]:
            self.browse_button.grid(row=0, column=2, padx=10, pady=5, sticky="ew")
        else:
            self.browse_button.grid_forget()

    def select_button(self, button_number):
        """This function is called when a button in the grid is pressed."""
        button_name = f"button{button_number}"
        self.update_active_button(button_name=button_name)

        # Load the macro into the form for editing
        if button_name in self.macros:
            macro = self.macros[button_name]
            self.value_entry.delete(0, tk.END)
            self.value_entry.insert(0, macro["value"])
            self.text_entry.delete(0, tk.END)
            self.text_entry.insert(0, macro["text"])
            self.set_modifier(macro["modifier"])

        # Highlight the corresponding macro in the TreeView
        for item in self.tree.get_children():
            values = self.tree.item(item, "values")
            if values[0] == button_name:
                self.tree.selection_set(item)
                self.tree.focus(item)
                self.tree.item(item, tags=("selected",))
                self.tree.tag_configure("selected", background="red")
            else:
                self.tree.item(item, tags=("unselected",))
                self.tree.tag_configure("unselected", background="white")

    def update_active_button(self, button_name=None):
        """Highlight the active button and reset the previously highlighted button."""
        if self.active_button and self.active_button != button_name:
            self.buttons[self.active_button].config(bg="SystemButtonFace")

        if button_name in self.buttons:
            self.buttons[button_name].config(bg="lightblue")
            self.active_button = button_name

    def add_macro(self):
        """Add or update the macro for the selected button."""
        button_name = self.active_button
        value = self.value_entry.get().strip()
        text = self.text_entry.get().strip()
        modifier = self.active_modifier

        if not button_name:
            messagebox.showerror("Error", "Please select a button.")
            return
        if not modifier:
            messagebox.showerror("Error", "Please select a modifier.")
            return

        self.macros[button_name] = {"value": value, "modifier": modifier, "text": text}
        self.refresh_treeview()

        self.value_entry.delete(0, tk.END)
        self.text_entry.delete(0, tk.END)
        self.active_modifier = None
        self.set_modifier("")

    def refresh_treeview(self):
        """Refresh the Treeview to display current macros."""
        for item in self.tree.get_children():
            self.tree.delete(item)
        for button, macro in self.macros.items():
            self.tree.insert("", "end", values=(button, macro["modifier"], macro["value"], macro["text"]))

    def save_json(self):
        """Save macros to a JSON file."""
        if not self.macros:
            messagebox.showwarning("No Macros", "No macros to save.")
            return

        file_path = filedialog.asksaveasfilename(defaultextension=".json", filetypes=[("JSON files", "*.json")], title="Save JSON File")
        if not file_path:
            return

        with open(file_path, 'w') as json_file:
            json.dump(self.macros, json_file, indent=4)

        messagebox.showinfo("Success", f"Macros saved to {file_path}")

    def load_json(self):
        """Load macros from a JSON file."""
        file_path = filedialog.askopenfilename(filetypes=[("JSON files", "*.json")], title="Open JSON File")
        if not file_path:
            return

        with open(file_path, 'r') as json_file:
            self.macros = json.load(json_file)

        self.refresh_treeview()

    def select_save_directory(self):
        """Select a directory for saving the JSON file."""
        self.save_directory = filedialog.askdirectory()
        if self.save_directory:
            messagebox.showinfo("Directory Selected", f"Default save directory: {self.save_directory}")

    def select_file_folder(self):
        """Open a file or folder selection dialog based on the modifier."""
        if self.active_modifier == "open":
            file_selected = filedialog.askopenfilename()
            if file_selected:
                self.value_entry.delete(0, tk.END)
                self.value_entry.insert(0, file_selected)
        elif self.active_modifier == "paste":
            folder_selected = filedialog.askdirectory()
            if folder_selected:
                self.value_entry.delete(0, tk.END)
                self.value_entry.insert(0, folder_selected)

    def on_treeview_double_click(self, event):
        """Handle double-clicking a macro in the Treeview."""
        selected_item = self.tree.selection()
        if selected_item:
            button_name = self.tree.item(selected_item, "values")[0]
            self.update_active_button(button_name=button_name)

            if button_name in self.macros:
                macro = self.macros[button_name]
                self.value_entry.delete(0, tk.END)
                self.value_entry.insert(0, macro["value"])
                self.text_entry.delete(0, tk.END)
                self.text_entry.insert(0, macro["text"])
                self.set_modifier(macro["modifier"])

            # Highlight the selected row in red
            for item in self.tree.get_children():
                self.tree.item(item, tags=("unselected",))
            self.tree.item(selected_item, tags=("selected",))
            self.tree.tag_configure("selected", background="red")
            self.tree.tag_configure("unselected", background="white")


def main():
    root = tk.Tk()
    app = MacroManagerApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()
