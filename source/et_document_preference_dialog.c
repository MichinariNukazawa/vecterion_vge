#include "et_document_preference_dialog.h"

#include "et_error.h"
#include "et_etaion.h"
#include "et_doc.h"

typedef struct{
	GtkWidget *spinbutton_snap_for_grid_width;
	GtkWidget *spinbutton_snap_for_grid_height;
}PvDocumentPreferenceDialog;

static bool pv_document_preference_dialog_set_ui_from_value(GtkBuilder *, const PvDocumentPreference *);
static bool pv_document_preference_dialog_get_value_from_ui(GtkBuilder *, PvDocumentPreference *);

static bool get_handle_(GtkBuilder *builder, PvDocumentPreferenceDialog *self)
{
	self->spinbutton_snap_for_grid_width
		= GTK_WIDGET(gtk_builder_get_object(builder, "spinbutton_snap_for_grid_width"));
	et_assert(self->spinbutton_snap_for_grid_width);

	self->spinbutton_snap_for_grid_height
		= GTK_WIDGET(gtk_builder_get_object(builder, "spinbutton_snap_for_grid_height"));
	et_assert(self->spinbutton_snap_for_grid_height);

	return true;
}

GtkBuilder *pv_document_preference_dialog_init(const char *application_base_dir)
{
	GtkBuilder *builder = gtk_builder_new();
	et_assert(builder);

	gchar *path = g_strdup_printf(
			"%s/resource/ui/document_preference_dialog.glade",
			application_base_dir);

	GError* error = NULL;
	if ( !gtk_builder_add_from_file(builder, path, &error)){
		et_critical("Couldn't load document_preference_dialog_builder file: %s", error->message);
		g_error_free(error);
		return NULL;
	}
	g_free(path);

	gtk_builder_connect_signals(builder, NULL);

	return builder;
}

int pv_document_preference_dialog_run(GtkBuilder *builder)
{
	et_assert(builder);

	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		et_bug("%d\n", doc_id);
		return GTK_RESPONSE_REJECT;
	}

	PvDocumentPreference document_preference = et_doc_gpv_document_preference_from_id(doc_id);

	GtkWidget *dialog = GTK_WIDGET(
			gtk_builder_get_object(
				builder,
				"document_preference_dialog"));
	et_assert(dialog);

	bool res;
	res = pv_document_preference_dialog_set_ui_from_value(
			builder,
			&document_preference);
	et_assert(res);

	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_hide (dialog);

	switch (result)
	{
		case GTK_RESPONSE_ACCEPT:
			{
				et_debug("Accept");
				res = pv_document_preference_dialog_get_value_from_ui(
						builder,
						&document_preference);
				et_assert(res);

				et_doc_spv_document_preference_from_id(doc_id, document_preference);
			}
			break;
		default:
			{
				et_debug("Cancel");
			}
			break;
	}
	return result;
}

static bool pv_document_preference_dialog_set_ui_from_value(
		GtkBuilder *builder,
		const PvDocumentPreference *document_preference)
{
	et_assert(builder);

	PvDocumentPreferenceDialog self_;
	PvDocumentPreferenceDialog *self = &self_;
	bool res = get_handle_(builder, self);
	et_assert(res);

	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(self->spinbutton_snap_for_grid_width),
			document_preference->snap_context.grid.x);
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(self->spinbutton_snap_for_grid_height),
			document_preference->snap_context.grid.y);

	return true;
}

static bool pv_document_preference_dialog_get_value_from_ui(
		GtkBuilder *builder,
		PvDocumentPreference *document_preference)
{
	et_assert(builder);
	et_assert(document_preference);

	PvDocumentPreferenceDialog self_;
	PvDocumentPreferenceDialog *self = &self_;
	bool res = get_handle_(builder, self);
	et_assert(res);

	document_preference->snap_context.grid.x
		= gtk_spin_button_get_value(
				GTK_SPIN_BUTTON(self->spinbutton_snap_for_grid_width));
	document_preference->snap_context.grid.y
		= gtk_spin_button_get_value(
				GTK_SPIN_BUTTON(self->spinbutton_snap_for_grid_height));

	return true;
}

