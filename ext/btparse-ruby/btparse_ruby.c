#include "ruby.h"
#include "btparse.h"

static VALUE rb_cBibTeX;
static VALUE rb_cEntry;

VALUE ast_to_class(AST *entries)
{
  VALUE rb_bibtex = rb_class_new_instance(0, NULL, rb_cBibTeX);
  VALUE rb_entries = rb_ary_new();

  AST *entry = NULL;
  while (entry = bt_next_entry(entries, entry))
  {
    /* Skip non-regular entries */
    if (bt_entry_metatype(entry) != BTE_REGULAR) {
      continue;
    }

    VALUE rb_entry = rb_class_new_instance(0, NULL, rb_cEntry);

    /* Set Entry type */
    char *entry_type = bt_entry_type(entry);
    if (entry_type != NULL)
    {
      rb_iv_set(rb_entry, "@type", rb_str_new2(entry_type));
    }

    /* Set Entry key */
    char *entry_key = bt_entry_key(entry);
    if (entry_key != NULL)
    {
      rb_iv_set(rb_entry, "@key", rb_str_new2(entry_key));
    }

    /* Set Entry fields */
    VALUE rb_fields = rb_hash_new();
    char *name = NULL;
    
    AST *field = NULL;
    while (field = bt_next_field(entry, field, &name))
    {
      char *value = bt_get_text(field);
      
      VALUE rb_name = rb_str_new2(name);
      VALUE rb_value  = rb_str_new2(value);
      
      rb_hash_aset(rb_fields, rb_name, rb_value);
    }
    
    rb_iv_set(rb_entry, "@fields", rb_fields);
    
    rb_ary_push(rb_entries, rb_entry);
  }

  rb_iv_set(rb_bibtex, "@entries", rb_entries);

  return rb_bibtex;
}


static VALUE bibtex_parse(VALUE klass, VALUE rb_entry_text)
{
  Check_Type(rb_entry_text, T_STRING);

  char *entry_text  = RSTRING_PTR(rb_entry_text);
  boolean status  = 0;

  bt_initialize();

  AST *entries = bt_parse_entry_s(entry_text, NULL, 1, 0, &status);

  /* Exit early if parsing failed */
  if (entries == NULL || status == 0)
  {
    return Qnil;
  }
  
  VALUE rb_entries = ast_to_class(entries);
  
  bt_parse_entry_s(NULL, NULL, 1, 0, NULL);
  
  bt_cleanup();
  
  return rb_entries;
}

static VALUE bibtex_parse_file(VALUE klass, VALUE rb_filename)
{
  Check_Type(rb_filename, T_STRING);

  char *filename  = RSTRING_PTR(rb_filename);
  boolean status  = 0;

  bt_initialize();

  AST *entries = bt_parse_file(filename, 0, &status);

  /* Exit early if parsing failed */
  if (entries == NULL || status == 0)
  {
    return Qnil;
  }

  VALUE rb_entries = ast_to_class(entries);

  bt_cleanup();

  return rb_entries;
}


static VALUE bibtex_parse_author(VALUE klass, VALUE rb_string)
{
  Check_Type(rb_string, T_STRING);

  bt_initialize();

  bt_stringlist *authors = bt_split_list(RSTRING_PTR(rb_string), "and", NULL, 1, "authors");

  VALUE rb_authors = rb_ary_new();

  for (int i = 0; i < authors->num_items; i++) {
    char *author = authors->items[i];

    VALUE rb_author = rb_str_new2(author);

    rb_ary_push(rb_authors, rb_author);
  }

  bt_free_list(authors);

  bt_cleanup();

  return rb_authors;
}

static VALUE bibtex_purify(VALUE klass, VALUE rb_string)
{
  Check_Type(rb_string, T_STRING);
  
  bt_initialize();
  
  char *string = malloc(RSTRING_LEN(rb_string) + 1);

  strcpy(string, RSTRING_PTR(rb_string));
  bt_purify_string(string, 0);

  bt_cleanup();

  return rb_str_new2(string);
}

void Init_btparse_ruby()
{
  rb_cBibTeX = rb_define_class("BibTeX", rb_cObject);
  rb_define_singleton_method(rb_cBibTeX, "parse", RUBY_METHOD_FUNC(bibtex_parse), 1);
  rb_define_singleton_method(rb_cBibTeX, "parse_file", RUBY_METHOD_FUNC(bibtex_parse_file), 1);
  rb_define_singleton_method(rb_cBibTeX, "parse_author", RUBY_METHOD_FUNC(bibtex_parse_author), 1);
  rb_define_singleton_method(rb_cBibTeX, "purify", RUBY_METHOD_FUNC(bibtex_purify), 1);
  
  rb_cEntry = rb_define_class_under(rb_cBibTeX, "Entry", rb_cObject);
}
