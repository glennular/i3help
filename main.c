#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

struct binding {
  gchar *keys;
  gchar *cmd;
  int type;
};

static gint opt_columns = 2;
static gchar *opt_filename = NULL;
static gint opt_maxtextlen = 100;
static gchar **opt_remaining = NULL;
static gint opt_maxrows = 50;

static GOptionEntry entries[] =
{
  { "col", 'c', 0, G_OPTION_ARG_INT, &opt_columns, "Number of columns", "N" },
  { "maxrows", 'r', 0, G_OPTION_ARG_INT, &opt_maxtextlen, "Max number of rows to draw", "N" },
  { "maxtextlen", 'l', 0, G_OPTION_ARG_INT, &opt_maxtextlen, "Max length of a text command", "N" },
  { "file", 'f', 0, G_OPTION_ARG_FILENAME, &opt_filename, "file to load instead of using i3-mesg", "M" },
  { G_OPTION_REMAINING, ' ', 0, G_OPTION_ARG_FILENAME_ARRAY, &opt_remaining, NULL, NULL},
  { NULL }
};

gchar* readProcess(){
  gchar *cmd = "i3-msg -t get_config";
  GError *err = NULL;
  gchar *result = NULL;

  if (!g_spawn_command_line_sync(cmd, &result, NULL, NULL, &err)){
    g_print("%s\n", err->message);
    exit(1);
  }

  return result;
}

gchar* readFile() {
  gssize length;
  gchar *content;
  GError *err = NULL;

  if (!g_file_get_contents (opt_filename, &content, &length, &err)) {
    g_print("%s\n", err->message);
    exit(1);
  }

  return content;
}

void strip_extra_spaces(gchar *str) {
  int i, x;
  for(i=x=0; str[i]; ++i)
    if(!g_ascii_isspace(str[i]) || (i > 0 && !g_ascii_isspace(str[i-1])))
      str[x++] = str[i];
  str[x] = '\0';
}

void print2(gchar *b) {
  g_print(b);
}

void print_binding(struct binding *b) {
  g_print("%s - %s\n", b->keys, b->cmd);
}

GSList *parse(gchar *all){
  gchar **line = NULL,
    **lines = NULL,
    **args = NULL;
  GSList *results = NULL;
  struct binding *t;
  gboolean inMode = FALSE,
           isModeTitle = FALSE;

  all = g_strdelimit(all, "\t", ' ');
  strip_extra_spaces(all);

  lines = g_strsplit_set(all, "\n", -1);

  int n_lines = g_strv_length (lines);
  for (int i = 0; i < n_lines; i++){
    if (strlen(lines[i]) > opt_maxtextlen) {
      lines[i] = g_strndup(lines[i], opt_maxtextlen);
    }
    line= g_strsplit_set(lines[i], " ", 3);

    if (line[0] == NULL) {
      continue;
    } else if (g_str_has_prefix(line[0], "mode")){
      inMode = TRUE;
      isModeTitle = TRUE;
    } else if (g_str_has_prefix(line[0], "}")){
        inMode = FALSE;
        continue;
    }
    else if (!g_str_has_prefix(line[0], "bindsym")){
      continue;
    }

    while (g_str_has_prefix(line[2], "--")){
        args = g_strsplit_set(line[2], " ", 2);
        line[1] = g_strdup_printf("%s %s", line[1], args[0]);
        line[2] = args[1];
    }
    if (g_str_has_prefix(line[1], "--")){
        args = g_strsplit_set(line[2], " ", 2);
        line[1] = g_strdup_printf("%s %s", line[1], args[0]);
        line[2] = args[1];
    }

    t = (struct binding*) malloc(sizeof(struct binding));

    //TODO: error checking on parsing
    t->type = isModeTitle ? 3 : inMode ? 2 : 1;
    t->keys = line[1];
    t->cmd = isModeTitle ? "" : line[2];

    results = g_slist_prepend(results, t);
    isModeTitle = FALSE;
  }
  return results;
}

gboolean keypress_function (GtkWidget *widget, GdkEventKey *event, gpointer data) {
    g_application_quit(G_APPLICATION(data));
    return TRUE;
}

void addLabel(GtkWidget *grid, int col, gchar* text){
  GtkWidget *label;
  label = gtk_label_new (NULL);

  gtk_widget_set_vexpand (label, TRUE);
  gtk_widget_set_valign (label, GTK_ALIGN_START);
  gtk_label_set_markup(GTK_LABEL(label), text);
  gtk_grid_attach (GTK_GRID (grid), label, col, 0, 1, 1);
}

static void activate (GtkApplication* app, gpointer user_data)
{
  GtkWidget *window;
  GtkWidget *grid;

  GSList *text = (GSList *) user_data;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "i3-help");
  gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

  gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
  g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (keypress_function), app);

  grid = gtk_grid_new ();

  gtk_widget_set_size_request(GTK_WIDGET(grid),800,600);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
  gtk_widget_set_margin_start(grid, 10);
  gtk_widget_set_margin_end(grid, 10);
  gtk_widget_set_margin_top(grid, 10);
  gtk_widget_set_margin_bottom(grid, 10);
  gtk_container_add (GTK_CONTAINER (window), grid);

  int col = 0;
  while(text != NULL){
    addLabel(grid, col++, text->data);
    text = g_slist_next(text);
  }

  gtk_widget_show_all (window);
}

char* replaceDelimiters(const gchar *text, const gchar *delims, const gchar *replacement)
{
  return g_strjoinv(replacement, g_strsplit(text, delims, -1));
}

gchar* clean(gchar *text){
  gchar* r = replaceDelimiters(text, "&", "&amp;");
  r = replaceDelimiters(r, "<", "&lt;");
  r = replaceDelimiters(r, ">", "&gt;");
  r = replaceDelimiters(r, "\"", "&quot;");
  r = replaceDelimiters(r, "'", "&#39;");
  return r;
}

gchar *formatKeys(gchar *text){
  return g_strdup_printf("<span foreground='#dd0'>%s</span>",
                         g_strjoinv("</span> + <span foreground='#dd0'>", g_strsplit(text, "+", -1)));
}

gchar *formatMode(int mode){
  return mode == 3 ? "<span>Mode</span> " : mode == 1 ? "" : "     ";
}

GSList* merge(GSList *lines){
  GSList *results = NULL;
  while(lines != NULL){
    struct binding *t = lines->data;
    results = g_slist_prepend(results, g_strdup_printf("%s%s : %s",
                                                      formatMode(t->type),
                                                      formatKeys(clean(t->keys)),
                                                      clean(t->cmd)));
    lines = g_slist_next(lines);
  }
  return results;
}

GSList* join(GSList *lines){
  GSList *results = NULL;
  gchar *result = "";
  gint total = g_slist_length(lines),
    colSize = MIN(opt_maxrows, (total / opt_columns)),
    colMod = total % opt_columns,
    row = 0,
    col = 0;

  while(lines != NULL){
    result = g_strdup_printf("%s%s", result, lines->data);
    lines = g_slist_next(lines);
    row++;
    if ((col >= colMod && row == colSize )
      || (col <= colMod && row == colSize)) {
      results = g_slist_append(results, result);
      result = "";
      row=0;
      col++;
    } else {
      result = g_strdup_printf("%s\n", result);
    }
  }

  return results;
}

void parseOptions(int argc, char *argv[]){
  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new ("- i3 keyboard shortcut help dialog");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_set_ignore_unknown_options (context, FALSE);

  if (!g_option_context_parse (context, &argc, &argv, &error))
  {
    g_print ("option parsing failed: %s\n", error->message);
    exit (1);
  }

  if (opt_remaining != NULL){
    if (opt_filename != NULL){
      g_print("Can not pass filename as default and -f arguments\n");
      exit(1);
    }
    opt_filename = opt_remaining[0];
  }

}

int main(int argc, char **argv) {
  int status;
  GtkApplication *app;
  gchar *all = NULL;
  GSList* lines = NULL;

  parseOptions(argc, argv);

  if (opt_filename == NULL){
    all = readProcess();
  } else {
    all = readFile();
  }

  lines = parse(all);
  lines = merge(lines);
  lines = join(lines);

  app = gtk_application_new (NULL, G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), lines);
  status = g_application_run (G_APPLICATION (app), 0, NULL);
  g_object_unref (app);

  return status;
}
