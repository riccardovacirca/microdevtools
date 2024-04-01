
#include "microdevtools.h"

/*
 * COMMON
 */

int ns_rand(int l, int h) {
  srand(time(NULL));
  return l < h ? (rand() % (h - l + 1)) + l : 0;
}

int ns_is_empty(const char *s) {
  int rv = 1;
  if (s && (*s != '\0')) {
    apr_size_t l = strlen(s);
    for (apr_size_t i = 0; i < l; i ++) {
      // The string is not empty if it contains at least one non-empty character
      if (!apr_isspace(s[i])) {
        rv = 0;
        break;
      }
    }
  }
  return rv;
}

int ns_is_int(const char *s) {
  int rv = 0;
  if (s && (*s != '\0')) {
    char *endp;
    (void)strtol(s, &endp, 10);
    rv = (endp != s) && (*endp == '\0');
  }
  return rv;
}

int ns_is_double(const char *s) {
  int rv = 0;
  if (s && (*s != '\0')) {
    char *endp;
    (void)strtod(s, &endp);
    rv = (endp != s) && (*endp == '\0');
  }
  return rv;
}

int ns_in_string(const char *s, const char *sub) {
  int rv = 0;
  if (s && sub) {
    apr_size_t ls, lsub;
    rv = ((ls = strlen(s)) > 0) && ((lsub = strlen(sub)) > 0) &&
         (lsub <= ls) && (strstr(s, sub) != NULL);
  }
  return rv;
}

// Allocates a string to a buffer of specified size
char *ns_buffer(apr_pool_t *mp, const char *s, apr_size_t *bf_size) {
  char *result = NULL, *ends = NULL, str[(*bf_size)+1];
  if (mp && s && *bf_size > 0) {
    ends = apr_cpystrn(str, s, (*bf_size)+1);
  }
  if (ends) {
    *bf_size = ends - str;
    if ((*bf_size) > 0) {
      result = (char*) apr_palloc(mp, sizeof(char)*(*bf_size) + 1);
      if (result) {
        ends = apr_cpystrn(result, s, (*bf_size) + 1);
      }
    }
  }
  if (!result) {
    *bf_size = 0;
  }
  // The returned string always has a NULL terminator and a size of
  // at most bf_size-1 bytes
  return result;
}

char *ns_str(apr_pool_t *mp, const char *s, apr_size_t sz) {
  char *result = NULL;
  if (mp && s && sz) {
    apr_size_t bf_size = sz;
    result = ns_buffer(mp, s, &bf_size);
  }
  return result;
}

char *ns_trim(apr_pool_t *mp, const char *s) {
  char *result = NULL;
  if (mp && s) {
    int start = 0, end = strlen(s) - 1;
    while (apr_isspace(s[start])) {
      start ++;
    }
    while ((end >= start) && apr_isspace(s[end])) {
      end --;
    }
    result = ns_str(mp, s + start, end - start + 1);
  }
  return result;
}

// char *ns_trim(apr_pool_t *pool, const char *str) {
//   int start = 0, end = strlen(str) - 1;
//   // Trova il primo carattere non vuoto dall'inizio della stringa
//   while (isspace((unsigned char)str[start])) {
//     start++;
//   }
//   // Trova l'ultimo carattere non vuoto dalla fine della stringa
//   while (end >= start && isspace((unsigned char)str[end])) {
//     end--;
//   }
//   // Alloca la memoria per la stringa trimmata
//   char *trimmed_str = apr_palloc(pool, end - start + 2);
//   // Copia i caratteri non vuoti nella nuova stringa
//   memcpy(trimmed_str, str + start, end - start + 1);
//   trimmed_str[end - start + 1] = '\0';
//   return trimmed_str;
// }

const char *ns_strip_char(apr_pool_t *mp, const char *s, char c) {
  char *result = NULL;
  apr_size_t l, j = 0;
  if (mp && s) {
    l = (apr_size_t)strlen(s);
    if (l > 0) {
      result = (char*)apr_palloc(mp, sizeof(char) * (l + 1));
    }
  }
  if (result) {
    // Rebuilds the string with every element different from c
    for (apr_size_t i = 0; i < l; i ++) {
      if (s[i] != c) {
        result[j] = s[i];
        j ++;
      }
    }
    result[j] = '\0';
  }
  return !result ? s : (const char*)result;
}

char *ns_slice(apr_pool_t *mp, const char *s, apr_size_t i, apr_size_t l) {
  char *result = NULL;
  apr_size_t len = 0;
  if (mp && s && (i >= 0) && (l > 0)) {
    len = (apr_size_t)strlen(s);
  }
  if ((len > 0) && (i <= (len - 1)) && (l <= (len - i))) {
    result = (char*)apr_palloc(mp, sizeof(char) * (l + 1));
  }
  if (result) {
    for (apr_size_t j = 0; j < l; j ++) {
      result[j] = s[i + j];
    }
    result[l] = '\0';
  }
  return result;
}

const char *ns_str_replace(apr_pool_t *mp, const char *s, const char *f, const char *r) {
  char *result = NULL;
  int i = 0, cnt = 0, r_len = 0, f_len = 0;
  if (mp && s && f && r) {
    if ((*s != '\0') && (*f != '\0') && (*r != '\0')) {
      if (strcmp(f, r) != 0) {
        f_len = strlen(f);
        r_len = strlen(r);
      }
    }
  }
  if (f_len > 0 && r_len > 0) {
    for (i = 0; s[i] != '\0'; i++) {
      if (strstr(&s[i], f) == &s[i]) {
        cnt ++;
        i += f_len - 1;
      }
    }
  }
  if (cnt > 0) {
    result = (char*)apr_palloc(mp, i + cnt * (r_len-f_len) + 1);
  }
  if (result) {
    i = 0;
    while (*s) {
      if (strstr(s, f) == s) {
        strcpy(&result[i], r);
        i += r_len;
        s += f_len;
      } else {
        result[i++] = *s++;
      }
    }
    result[i] = '\0';
  }
  return !result ? s : (const char*)result;
}

const char *ns_replace_char(apr_pool_t *mp, const char *s, char f, char r) {
  char *result = NULL;
  if (mp && s && f && r) {
    if((*s != '\0') && (f != r)) {
      result = apr_pstrdup(mp, s);
    }
  }
  if (result) {
    for (int i = 0; i < strlen(result); i++) {
      if (result[i] == f) {
        result[i] = r;
      }
    }
  }
  return !result ? s : (const char*)result;
}

char *ns_empty_string_make(apr_pool_t *mp) {
  char *result = NULL;
  if (mp) {
    result = (char*)apr_palloc(mp, 1);
  }
  if (result) {
    result[0] = '\0';
  }
  return result;
}

apr_array_header_t* ns_split(apr_pool_t *mp, const char *s, const char *sp)
{
  apr_array_header_t *result = NULL;
  char *str = NULL;
  const char *tmp = NULL;
  apr_size_t l_sp = 0;
  if (mp && s && sp) {
    if (strlen(s) > 0) {
      l_sp = (apr_size_t)strlen(sp);
    }
  }
  if(l_sp > 0) {
    result = apr_array_make(mp, 0, sizeof(const char*));
  }
  if (result) {
    str = apr_pstrdup(mp, s);
  }
  if (str) {
    char *ptr = strstr(str, sp);
    while (ptr) {
      *ptr = '\0';
      if (strlen(str) <= 0) {
        tmp = (const char*)ns_empty_string_make(mp);
        if (tmp) {
          APR_ARRAY_PUSH(result, const char*) = tmp;
        }
      } else {
        tmp = apr_pstrdup(mp, str);
        if (tmp) {
          APR_ARRAY_PUSH(result, const char*) = tmp;
        }
      }
      str = ptr + l_sp;
      ptr = strstr(str, sp);
    }
  }
  if (strlen(str) <= 0) {
    tmp = (const char*)ns_empty_string_make(mp);
    if (tmp) {
      APR_ARRAY_PUSH(result, const char*) = tmp;
    }
  } else {
    tmp = apr_pstrdup(mp, str);
    if (tmp) {
      APR_ARRAY_PUSH(result, const char*) = tmp;
    }
  }
  return result;
}

char *ns_join(apr_pool_t *mp, apr_array_header_t *a, const char *sp)
{
  int valid_input = 0, valid_array = 0;
  apr_size_t sp_l;
  char *item, *result = NULL;
  apr_array_header_t *tmp = NULL;
  valid_input = mp && a;
  if (valid_input) {
    valid_array = a->nelts > 0;
  }
  if (valid_array) {
    if (!sp) {
      result = apr_array_pstrcat(mp, a, 0);
    } else {
      sp_l = (apr_size_t)strlen(sp);
      if (sp_l > 0) {
        for (int i = 0; i < a->nelts; i ++) {
          item = APR_ARRAY_IDX(a, i, char*);
          if (item) {
            if (!tmp) {
              tmp = apr_array_make(mp, a->nelts, sizeof(char*));
            }
          }
          if (tmp) {
            APR_ARRAY_PUSH(tmp, char*) = apr_pstrdup(mp, item);
            if (i < (a->nelts - 1)) {
              APR_ARRAY_PUSH(tmp, char*) = apr_pstrdup(mp, sp);
            }
          }
        }
      }
      if (tmp && (tmp->nelts > 0)) {
        result = apr_array_pstrcat(mp, tmp, 0);
      }
    }
  }
  return result;
}

char *ns_md5(apr_pool_t *mp, const char *s)
{
  char *result = NULL;
  apr_size_t l = 0;
  unsigned char digest[APR_MD5_DIGESTSIZE];
  if (mp && s) { 
    l = strlen(s);
  }
  if(l > 0) {
    apr_md5_ctx_t ctx;
    apr_md5_init(&ctx);
    apr_md5_update(&ctx, s, l);
    apr_md5_final(digest, &ctx);
    result = (char*)apr_pcalloc(mp, 32 + 1);
  }
  if (result) {
    for (int i = 0; i < APR_MD5_DIGESTSIZE; i ++) {
      sprintf(&result[i * 2], "%02x", digest[i]);
    }
  }
  return result;
}

char *ns_base64_encode(apr_pool_t *mp, const char *s)
{
  char *result = NULL;
  apr_size_t l = 0;
  if (mp && s) {
    l = (apr_size_t)strlen(s);
  }
  if (l > 0) {
    result = (char*)apr_pcalloc(mp, apr_base64_encode_len(l));
  }
  if (result != NULL) {
    apr_base64_encode(result, s, l);
  }
  return result;
}

char *ns_base64_decode(apr_pool_t* mp, const char *s)
{
  char *result = NULL;
  apr_size_t s_len = 0, max_rv_len = 0, rv_len = 0;
  if (mp && s) {
    s_len = strlen(s);
  }
  if (s_len > 0) {
    max_rv_len = apr_base64_decode_len(s);
  }
  if (max_rv_len > 0) {
    result = (char*)apr_palloc(mp, max_rv_len);
  }
  if (result) {
    rv_len = apr_base64_decode(result, s);
  }
  if (rv_len >= 0) {
    result[rv_len] = '\0';
  }
  return result;
}

apr_table_t* ns_args_to_table(apr_pool_t *mp, const char *q)
{
  apr_table_t *result = NULL;
  apr_array_header_t *args, *elts;
  args = ns_split(mp, q, "&");
  if (args && args->nelts) {
    result = apr_table_make(mp, args->nelts);
    for (int i = 0; i < args->nelts; i++) {
      const char *arg = APR_ARRAY_IDX(args, i, const char*);
      elts = ns_split(mp, arg, "=");
      if (elts && elts->nelts == 2) {
        apr_table_set(
          result,
          APR_ARRAY_IDX(elts, 0, const char*),
          APR_ARRAY_IDX(elts, 1, const char*)
        );
      }
    }
  }
  return result;
}

int ns_table_nelts(apr_table_t *t)
{
  return t ? (apr_table_elts(t))->nelts : -1;
}

apr_table_entry_t* ns_table_elt(apr_table_t *t, int i)
{
  apr_table_entry_t *result = NULL;
  if (t && (i >= 0)) {
    if (i < (apr_table_elts(t))->nelts) {
      result = &((apr_table_entry_t*)((apr_table_elts(t))->elts))[i];
    }
  }
  return result;
}

char *ns_datetime(apr_pool_t *mp, apr_time_t t, const char *f)
{
  char *result = NULL;
  apr_time_exp_t tm;
  apr_size_t size = 100;
  const char *fm = NULL;
  char tmp[100] = {0};
  if (mp && t && f) {
    if (apr_time_exp_lt(&tm, t) == APR_SUCCESS) {
      fm = apr_pstrdup(mp, f);
      if (fm) {
        fm = ns_str_replace(mp, ns_str_replace(mp, fm, "Y", "%Y"), "y", "%y");
        fm = ns_str_replace(mp, ns_str_replace(mp, fm, "m", "%m"), "d", "%d");
        fm = ns_str_replace(mp, ns_str_replace(mp, fm, "H", "%H"), "h", "%I");
        fm = ns_str_replace(mp, ns_str_replace(mp, fm, "s", "%S"), "i", "%M");
      }
    }
  }
  if (fm) {
    if (apr_strftime(tmp, &size, 100, fm, &tm) == APR_SUCCESS) {
      result = apr_pstrdup(mp, tmp);
    }
  }
  return result;
}

char *ns_datetime_local(apr_pool_t *mp, apr_time_t t, const char *f)
{
  char *result = NULL;
  apr_time_exp_t tm;
  apr_size_t size = 100;
  const char *fm = NULL;
  char tmp[100] = {0};
  if (mp && t && f) {
    if (apr_time_exp_lt(&tm, t) == APR_SUCCESS) {
      fm = apr_pstrdup(mp, f);
      if (fm) {
        fm = ns_str_replace(mp, ns_str_replace(mp, fm, "Y", "%Y"), "y", "%y");
        fm = ns_str_replace(mp, ns_str_replace(mp, fm, "m", "%m"), "d", "%d");
        fm = ns_str_replace(mp, ns_str_replace(mp, fm, "H", "%H"), "h", "%I");
        fm = ns_str_replace(mp, ns_str_replace(mp, fm, "s", "%S"), "i", "%M");
        fm = apr_pstrcat(mp, fm, "%z", NULL);
      }
    }
  }
  if (fm) {
    if (apr_strftime(tmp, &size, 100, fm, &tm) == APR_SUCCESS) {
      result = apr_pstrdup(mp, tmp);
    }
  }
  return result;
}

char *ns_datetime_utc(apr_pool_t *mp, apr_time_t t, const char *f)
{
  apr_time_exp_t tm;
  apr_size_t size = 100;
  char tmp[100] = {0}, *result = NULL;
  if (mp && t) {
    // Usa apr_time_exp_gmt invece di apr_time_exp_lt
    if (apr_time_exp_gmt(&tm, t) == APR_SUCCESS) {
      // Formato desiderato
      const char *fm = "%Y-%m-%d %H:%M:%S";
      if (apr_strftime(tmp, &size, 100, fm, &tm) == APR_SUCCESS) {
        result = apr_pstrdup(mp, tmp);
      }
    }
  }
  return result;
}

int ns_is_dir(const char *d, apr_pool_t *mp)
{
  apr_finfo_t finfo;
  return mp && d && (strlen(d) > 0) &&
    (apr_stat(&finfo, d, APR_FINFO_TYPE, mp) == APR_SUCCESS) &&
    (finfo.filetype == APR_DIR);
}

int ns_is_file(const char *f, apr_pool_t *mp)
{
  apr_finfo_t finfo;
  return mp && f && (strlen(f) > 0) &&
    (apr_stat(&finfo, f, APR_FINFO_NORM, mp) == APR_SUCCESS);
}

apr_status_t ns_file_open(apr_file_t **fd, const char *f, apr_int32_t fl, apr_pool_t *mp)
{
  apr_status_t result = APR_EGENERAL;
  if (mp && f) {
    result = apr_file_open(fd, f, fl, APR_OS_DEFAULT, mp);
  }
  return result;
}

apr_status_t ns_file_open_read(apr_file_t **fd, const char *f, apr_pool_t *mp)
{
  return ns_file_open(fd, f, APR_READ, mp);
}

apr_status_t ns_file_open_append(apr_file_t **fd, const char *f, apr_pool_t *mp)
{
  return ns_file_open(fd, f, APR_WRITE | APR_CREATE | APR_APPEND, mp);
}

apr_status_t ns_file_open_truncate(apr_file_t **fd, const char *f,
                                   apr_pool_t *mp)
{
  return ns_file_open(fd, f, APR_WRITE | APR_CREATE | APR_TRUNCATE, mp);
}

apr_size_t ns_file_write(apr_file_t *fd, const char *buf, apr_size_t l)
{
  apr_size_t result = 0;
  if (fd && buf && (l > 0)) {
    apr_status_t st = apr_file_write_full(fd, buf, l, &result);
    if (st != APR_SUCCESS) {
      result = 0;
    }
  }
  return result;
}

apr_size_t ns_file_read(apr_pool_t *mp, apr_file_t *fd, void **buf)
{
  apr_size_t result = 0;
  if (mp && fd && buf) {
    apr_finfo_t finfo;
    apr_status_t st = apr_file_info_get(&finfo, APR_FINFO_NORM, fd);
    apr_size_t fsize = 0;
    if (st == APR_SUCCESS) {
      fsize = (apr_size_t)finfo.size;
    }
    if (fsize <= 0) {
      *buf = NULL;
    } else {
      if (fsize > NS_MAX_READ_BUFFER) {
        fsize = NS_MAX_READ_BUFFER;
      }
      *buf = (void*)apr_palloc(mp, fsize);
      if (buf) {
        st = apr_file_read_full(fd, *buf, fsize, &result);
      }
    }
  }
  return result;
}

apr_status_t ns_file_close(apr_file_t *fd)
{
  return apr_file_close(fd);
}

apr_time_t ns_timestamp(int year, int month, int day, int hour,
                        int minute, int second)
{
  if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
    return apr_time_now();
  }
  if (year < 1970 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31 ||
    hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
    return NS_ERROR_TIMESTAMP;
  }
  apr_time_exp_t timeExp;
  apr_time_t currentTime = apr_time_now(); // Ottieni il tempo corrente
  apr_time_exp_gmt(&timeExp, currentTime); // Inizializza la struttura con il tempo corrente
  timeExp.tm_year = year - 1900;  // Anno - 1900
  timeExp.tm_mon = month - 1;    // Mese (da 0 a 11)
  timeExp.tm_mday = day;         // Giorno del mese
  timeExp.tm_hour = hour;        // Ora del giorno
  timeExp.tm_min = minute;       // Minuto
  timeExp.tm_sec = second;       // Secondo
  timeExp.tm_usec = 0;           // Microsecondo
  apr_time_t unixTime;
  apr_time_exp_gmt_get(&unixTime, &timeExp);
  return unixTime;
}

apr_time_t ns_now()
{
  return ns_timestamp(0, 0, 0, 0, 0, 0);
}

apr_table_entry_t* ns_table_entry(apr_table_t *t, int i)
{
  return (t != NULL) && (i >= 0) && (i < (apr_table_elts(t))->nelts)
    ? &((apr_table_entry_t*)((apr_table_elts(t))->elts))[i]
    : NULL;
}

// Legge i dati dallo standard input e li restituisce come una stringa.
// 'm' è il pool di memoria da utilizzare per l'allocazione di eventuali risorse.
char *ns_pipein(apr_pool_t *mp)
{
  char *result = NULL;
  char buf[NS_MAX_READ_BUFFER] = {0};
  apr_size_t l;
  apr_file_t *fd;
  apr_size_t bytes = NS_MAX_READ_BUFFER - 1;
  apr_status_t st = apr_file_open_stdin(&fd, mp);
  if (st == APR_SUCCESS) {
    st = apr_file_read(fd, buf, &bytes);
  }
  if (st == APR_SUCCESS) {
    if (bytes > 0) {
      result = (char*)apr_palloc(mp, bytes + 1);
    }
    if (result) {
      memcpy(result, buf, bytes);
      result[bytes] = '\0';
    }
    apr_file_close(fd);
  }
  return result;
}

char *ns_env(const char *e, apr_pool_t *mp)
{
  char *result;
  return mp && e && (apr_env_get(&result, e, mp) == APR_SUCCESS) ? result : NULL;
}

void ns_daemonize()
{
  pid_t pid, sid;
  pid = fork();
  if (pid < 0) {
    perror("Fork failed");
    exit(1);
  }
  if (pid > 0) {
    exit(0);
  }
  sid = setsid();
  if (sid < 0) {
    perror("Error creating new session");
    exit(1);
  }
  pid = fork();
  if (pid < 0) {
    perror("Second fork failed");
    exit(1);
  }
  if (pid > 0) {
    exit(0);
  }
  if (chdir("/") < 0) {
    perror("Error changing working directory");
    exit(1);
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}

/*
 * JSON
 */

ns_json_pair_t* ns_json_pair_init(apr_pool_t *mp) {
  ns_json_pair_t *result = NULL;
  if (mp != NULL) {
    if ((result = (ns_json_pair_t*)apr_palloc(mp, sizeof(ns_json_pair_t))) != NULL) {
      //result->pool = mp;
      result->key = NULL;
      result->val = NULL;
      result->type = NS_JSON_T_ZERO;
    }
  }
  return result;
}

ns_json_object_t* ns_json_object_init(apr_pool_t *mp) {
  return (ns_json_object_t*)apr_array_make(mp, 0, sizeof(ns_json_pair_t*));
}

int ns_json_object_add(apr_pool_t *mp, ns_json_object_t *jo, ns_json_type_t tp,
                       const char *k, void *v)
{
  int result = 0;
  ns_json_pair_t *entry;
  if ((mp != NULL) && (jo != NULL) && (tp >= 0)) {
    if ((entry = ns_json_pair_init(mp)) != NULL) {
      entry->key = k;
      entry->val = v;
      entry->type = tp;
      APR_ARRAY_PUSH(jo, ns_json_pair_t*) = entry;
      result = 1;
    }
  }
  return result;
}

ns_json_type_t ns_int_type(apr_int64_t v)
{
  if (v < APR_INT32_MIN) {
    return NS_JSON_T_INT64;
  } else if (v < APR_INT16_MIN) {
    return NS_JSON_T_INT32;
  } else if (v <= APR_INT16_MAX) {
    return NS_JSON_T_INT16;
  } else if (v <= APR_UINT16_MAX) {
    return NS_JSON_T_UINT16;
  } else if (v <= APR_INT32_MAX) {
    return NS_JSON_T_INT32;
  } else if (v <= APR_UINT32_MAX) {
    return NS_JSON_T_UINT32;
  } else if (v < APR_INT64_MAX) {
    return NS_JSON_T_INT64;
  } else {
    return NS_JSON_T_ZERO;
  }
}

ns_json_pair_t* ns_json_array_entry_make(apr_pool_t *mp, int type,
                                         const char *key, json_object *val)
{
  ns_json_pair_t *entry = ns_json_pair_init(mp);
  entry->key = key != NULL ? apr_pstrdup(mp, key) : NULL;
  // Eseguo lo switch dei tipi predefiniti di json-c
  switch (type) {
    case json_type_null: {
      entry->type = NS_JSON_T_NULL;
      entry->val = NULL;
    } break;
    case json_type_boolean:{
      entry->type = NS_JSON_T_BOOLEAN;
      entry->val = (void*)apr_palloc(mp, sizeof(char));
      *((char*)entry->val) = json_object_get_boolean(val);
    } break;
    case json_type_double: {
      entry->type = NS_JSON_T_DOUBLE;
      entry->val = (void*)apr_palloc(mp, sizeof(double));
      *((double*)entry->val) = json_object_get_double(val);
    } break;
    case json_type_int: {
      apr_uint64_t tmp_u = 0;
      apr_int64_t tmp_i = (apr_int64_t)json_object_get_int64(val);
      ns_json_type_t int_type = ns_int_type(tmp_i);
      if (!int_type) {
        tmp_u = (apr_uint64_t)json_object_get_uint64(val);
        if (tmp_u > APR_INT64_MAX) {
          int_type = NS_JSON_T_UINT64;
        } else {
          int_type = NS_JSON_T_INT64;
        }
      }
      if (int_type == NS_JSON_T_INT16) {
        entry->type = NS_JSON_T_INT16;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_int16_t));
        *((apr_int16_t*)entry->val) = (apr_int16_t)tmp_i;
      } else if (int_type == NS_JSON_T_UINT16) {
        entry->type = NS_JSON_T_UINT16;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_uint16_t));
        *((apr_uint16_t*)entry->val) = (apr_uint16_t)tmp_i;
      } else if (int_type == NS_JSON_T_INT32) {
        entry->type = NS_JSON_T_INT32;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_int32_t));
        *((apr_int32_t*)entry->val) = (apr_int32_t)tmp_i;
      } else if (int_type == NS_JSON_T_UINT32) {
        entry->type = NS_JSON_T_UINT32;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_uint32_t));
        *((apr_uint32_t*)entry->val) = (apr_uint32_t)tmp_i;
      } else if (int_type == NS_JSON_T_INT64) {
        entry->type = NS_JSON_T_INT64;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_int64_t));
        *((apr_int64_t*)entry->val) = (apr_int64_t)tmp_i;
      } else if (int_type == NS_JSON_T_UINT64) {
        entry->type = NS_JSON_T_UINT64;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_uint64_t));
        *((apr_uint64_t*)entry->val) = (apr_uint64_t)tmp_u;
      }
    } break;
    case json_type_string: {
      entry->type = NS_JSON_T_STRING;
      entry->val = (void*)apr_pstrdup(mp, (const char*)json_object_get_string(val));
    } break;
  }
  return entry;
}

apr_array_header_t* ns_json_parse(apr_pool_t *mp, json_object *jobj);

apr_array_header_t* ns_json_parse_array(apr_pool_t *mp, json_object *jarr)
{
  int jarr_l;
  enum json_type type;
  //, *jtmp; è stata sostituita dalla seguente riga:
  json_object *jval; 
  ns_json_pair_t *entry;
  apr_array_header_t *res = NULL;
  jarr_l = json_object_array_length(jarr);
  for (int i = 0; i < jarr_l; i ++) {
    jval = json_object_array_get_idx(jarr, i);
    type = json_object_get_type(jval);
    if (type == json_type_array) {
      entry = ns_json_pair_init(mp);
      entry->type = NS_JSON_T_ARRAY;
      entry->key = NULL;
      entry->val = (void*)ns_json_parse_array(mp, jval);
    } else if (type == json_type_object) {
      //entry = (ns_json_pair_t*)apr_palloc(mp, sizeof(ns_json_pair_t));
      entry = ns_json_pair_init(mp);
      entry->type = NS_JSON_T_OBJECT;
      entry->key = NULL;
      entry->val = (void*)ns_json_parse(mp, jval);
    } else {
      entry = ns_json_array_entry_make(mp, type, NULL, jval);
    }
    if (res == NULL) res = apr_array_make(mp, 0, sizeof(ns_json_pair_t*));
    APR_ARRAY_PUSH(res, ns_json_pair_t*) = entry;
  }
  return res;
}

apr_array_header_t* ns_json_parse(apr_pool_t *mp, json_object *jobj)
{
  ns_json_pair_t *entry = NULL;
  apr_array_header_t *res = NULL;
  enum json_type type;
  json_object *jtmp;
  json_object_object_foreach(jobj, key, val) {
    type = json_object_get_type(val);
    switch (type) {
      case json_type_object: {
        if (json_object_object_get_ex(jobj, key, &jtmp)) {
          entry = ns_json_pair_init(mp);
          entry->type = NS_JSON_T_OBJECT;
          entry->key = apr_pstrdup(mp, key);
          entry->val = (void*)ns_json_parse(mp, jtmp);
        }
      } break;
      case json_type_array: {
        if (json_object_object_get_ex(jobj, key, &jtmp)) {
          entry = ns_json_pair_init(mp);
          entry->type = NS_JSON_T_ARRAY;
          entry->key = apr_pstrdup(mp, key);
          entry->val = (void*)ns_json_parse_array(mp, jtmp);
        }
      } break;
      default: {
        entry = ns_json_array_entry_make(mp, type, key, val);
      } break;
    }
    if (res == NULL) res = apr_array_make(mp, 0, sizeof(ns_json_pair_t*));
    APR_ARRAY_PUSH(res, ns_json_pair_t*) = entry;
  }
  return res;
}

apr_array_header_t* ns_json_decode(apr_pool_t *mp, const char *s)
{
  json_object *jobj;
  apr_array_header_t* result;
  jobj = json_tokener_parse(s);
  result = ns_json_parse(mp, jobj);
  json_object_put(jobj);
  return result;
}

const char *ns_json_encode(apr_pool_t *mp, const void *v, ns_json_type_t tp)
{
  int len;
  apr_table_entry_t *e;
  apr_table_t *t;
  ns_json_pair_t *p;
  // Dichiaro 2 array temporanei
  apr_array_header_t *obj, *arr = NULL;
  // Inizializzo il valore di ritorno
  const char *result = NULL;
  // Verifico che la memoria sia allocata e il tipo di dato specificato
  if (mp != NULL && tp) {
    if (v == NULL || tp == NS_JSON_T_NULL) {
      // Il dato è una primitiva NULL
      result = apr_pstrdup(mp, NS_JSON_NULL_S);
    } else if (tp == NS_JSON_T_BOOLEAN) {
      // Il dato è una primitiva booleana
      result = apr_pstrdup(mp, *(char*)v ? NS_JSON_TRUE_S : NS_JSON_FALSE_S);
    } else if (tp == NS_JSON_T_INT16) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%hd", *((apr_int16_t*)v));
    } else if (tp == NS_JSON_T_UINT16) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%hu", *((apr_uint16_t*)v));
    } else if (tp == NS_JSON_T_INT32) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%d", *((apr_int32_t*)v));
    } else if (tp == NS_JSON_T_UINT32) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%u", *((apr_uint32_t*)v));
    } else if (tp == NS_JSON_T_INT64) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%" APR_INT64_T_FMT, *((apr_int64_t*)v));
    } else if (tp == NS_JSON_T_UINT64) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%" APR_UINT64_T_FMT, *((apr_uint64_t*)v));
    } else if (tp == NS_JSON_T_DOUBLE) {
      // Il dato è una primitiva double
      result = apr_psprintf(mp, "%0.8lf", *(double*)v);
    } else if (tp == NS_JSON_T_STRING) {
      // Il dato è una stringa
      result = apr_psprintf(mp, "\"%s\"", apr_pescape_echo(mp, (const char*)v, 1));
    } else if (tp == NS_JSON_T_JSON) {
      // Il dato è una stringa JSON pre-codificata
      result = apr_psprintf(mp, "%s", (const char*)v);
    } else if (tp == NS_JSON_T_TIMESTAMP) {
      // Il dato è un apr_time_t
      result = apr_psprintf(mp, "%" APR_TIME_T_FMT, (apr_time_t)v);
    } else if (tp > NS_JSON_T_VECTOR) {
      // Il dato è un vettore di elementi di tipo (tp - NS_JSON_T_VECTOR)
      // La funzione si aspetta un vettore di primitive o di stringhe
      int type = tp - NS_JSON_T_VECTOR;
      // Un vettore è una struttura apr_array_header_t di dati dello stesso tipo
      obj = (apr_array_header_t*)v;
      // Verifico che la struttura non sia vuota
      if (obj->nelts > 0) {
        if (arr == NULL) {
          // Alloco un array temporaneo per gli elementi del vettore
          arr = apr_array_make(mp, 1, sizeof(const char*));
        }
        if (arr != NULL) {
          // Ripeto per ogni elemento del vettore
          for (int i = 0; i < obj->nelts; i ++) {
            switch (type) {
              case NS_JSON_T_NULL: {
                // Aggiungo all'array temporaneo una stringa null
                APR_ARRAY_PUSH(arr, const char*) = apr_pstrdup(mp, NS_JSON_NULL_S);
              } break;
              case NS_JSON_T_BOOLEAN: {
                // Estraggo il intero
                int entry = APR_ARRAY_IDX(obj, i, int);
                // Aggiungo all'array temporaneo una stringa true o false
                APR_ARRAY_PUSH(arr, const char*) = apr_pstrdup(mp, entry ? NS_JSON_TRUE_S : NS_JSON_FALSE_S);
              } break;
              case NS_JSON_T_INT16: {
                // Estraggo il valore intero
                apr_int16_t entry = APR_ARRAY_IDX(obj, i, apr_int16_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%hd", entry);
              } break;
              case NS_JSON_T_UINT16: {
                // Estraggo il valore intero
                apr_uint16_t entry = APR_ARRAY_IDX(obj, i, apr_uint16_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%hu", entry);
              } break;
              case NS_JSON_T_INT32: {
                // Estraggo il valore intero
                apr_int32_t entry = APR_ARRAY_IDX(obj, i, apr_int32_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%d", entry);
              } break;
              case NS_JSON_T_UINT32: {
                // Estraggo il valore intero
                apr_uint32_t entry = APR_ARRAY_IDX(obj, i, apr_uint32_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%u", entry);
              } break;
              case NS_JSON_T_INT64: {
                // Estraggo il valore intero
                apr_int64_t entry = APR_ARRAY_IDX(obj, i, apr_int64_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%" APR_INT64_T_FMT, entry);
              } break;
              case NS_JSON_T_UINT64: {
                // Estraggo il valore intero
                apr_uint64_t entry = APR_ARRAY_IDX(obj, i, apr_uint64_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%" APR_UINT64_T_FMT, entry);
              } break;
              case NS_JSON_T_DOUBLE: {
                // Estraggo il valore double
                double entry = APR_ARRAY_IDX(obj, i, double);
                // Aggiungo all'array temporaneo il valore double
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%0.8lf", entry);
              } break;
              case NS_JSON_T_STRING: {
                // Estraggo il valore stringa
                // ------------------------------------------------------------
                // FIXME: deve essere eseguito l'escape della stringa estratta
                //        prima che venga aggiunta all'array temporaneo
                // ------------------------------------------------------------
                const char *entry = APR_ARRAY_IDX(obj, i, const char*);
                // Aggiungo all'array temporaneo il valore stringa
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\"", apr_pescape_echo(mp, entry, 1));
              } break;
              case NS_JSON_T_JSON: {
                const char *entry = APR_ARRAY_IDX(obj, i, const char*);
                // Aggiungo all'array temporaneo il valore stringa JSON
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%s", entry);
              } break;
              case NS_JSON_T_TIMESTAMP: {
                // Estraggo il valore apr_time_t
                apr_time_t entry = APR_ARRAY_IDX(obj, i, apr_time_t);
                // Aggiungo all'array temporaneo il valore apr_time_t
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%" APR_TIME_T_FMT, entry);
              } break;
              case NS_JSON_T_TABLE: {
                apr_table_t *entry = APR_ARRAY_IDX(obj, i, apr_table_t*);
                APR_ARRAY_PUSH(arr, const char*) =
                  //apr_psprintf(mp, "\"%s\"", apr_pescape_echo(mp, entry, 1));
                  ns_json_encode(mp, (const void*)entry, NS_JSON_T_TABLE);
              } break;
            }
          }
          // Al termine del ciclo for se l'array temporaneo non è vuoto
          // setto il valore di ritorno con la sua versione serializzata
          // in caso contrario il valore di ritorno contiene ancora NULL
          if (arr->nelts > 0) {
            const char *tmp_s = ns_join(mp, arr, ",");
            if (tmp_s != NULL) {
              result = apr_psprintf(mp, "[%s]", tmp_s);
            }
            // @todo else
          }
        }
      }
    } else if (tp == NS_JSON_T_TABLE) {
      t = (apr_table_t*)v;
      if (t && (len = (apr_table_elts(t))->nelts)) {
        if ((arr = apr_array_make(mp, len, sizeof(const char*)))) {
          for (int i = 0; i < len; i ++) {
            if ((e = &((apr_table_entry_t*)((apr_table_elts(t))->elts))[i])) {
              APR_ARRAY_PUSH(arr, const char*) =
                apr_psprintf(mp, "\"%s\":\"%s\"", (const char*)e->key,
                             apr_pescape_echo(mp, (const char*)e->val, 1));
            }
          }
          if (arr->nelts > 0) {
            const char *tmp_s = ns_join(mp, arr, ",");
            if (tmp_s != NULL) {
              result = apr_psprintf(mp, "{%s}", tmp_s);
            }
          }
        }
      }
    } else if (tp == NS_JSON_T_OBJECT) {
      // Il dato è un oggetto (ovvero un array associativo)
      // Un oggetto è una struttura apr_array_header_t di ns_json_pair_t
      // La struttura ns_json_pair_t contiene informazioni anche sul tipo di dato
      // La funzione richiede che le chiavi dei pair dell'array non siano NULL
      // altrimenti l'elemento non verrà aggiunto all'array temporaneo
      obj = (apr_array_header_t*)v;
      // Verifico che l'oggetto non sia vuoto
      if (obj->nelts > 0) {
        // Alloco un array temporaneo per gli elementi dell'oggetto
        if ((arr = apr_array_make(mp, 1, sizeof(const char*))) != NULL) {
          // Ripeto per ogni elemento dell'oggetto
          for (int i = 0; i < obj->nelts; i++) {
            // Estraggo il prossimo pair
            if ((p = APR_ARRAY_IDX(obj, i, ns_json_pair_t*)) != NULL) {
              if (!p->key) continue;
              switch (p->type) {
                case NS_JSON_T_NULL: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore null
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%s", p->key, NS_JSON_NULL_S);
                } break;
                case NS_JSON_T_BOOLEAN: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore boolean
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%s", p->key, *(char*)p->val ? NS_JSON_TRUE_S : NS_JSON_FALSE_S);
                } break;
                case NS_JSON_T_INT16: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%hd", p->key, *(apr_int16_t*)p->val);
                } break;
                case NS_JSON_T_UINT16: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%hu", p->key, *(apr_uint16_t*)p->val);
                } break;
                case NS_JSON_T_INT32: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%d", p->key, *(apr_int32_t*)p->val);
                } break;
                case NS_JSON_T_UINT32: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%u", p->key, *(apr_uint32_t*)p->val);
                } break;
                case NS_JSON_T_INT64: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%" APR_INT64_T_FMT, p->key, *(apr_int64_t*)p->val);
                } break;
                case NS_JSON_T_UINT64: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%" APR_UINT64_T_FMT, p->key, *(apr_uint64_t*)p->val);
                } break;
                case NS_JSON_T_TIMESTAMP: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore timestamp
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%" APR_TIME_T_FMT, p->key, *(apr_time_t*)p->val);
                } break;
                case NS_JSON_T_DOUBLE: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore double
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%0.8lf", p->key, *(double*)p->val);
                } break;
                case NS_JSON_T_STRING: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore string
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":\"%s\"", p->key, apr_pescape_echo(mp, (const char*)p->val, 1));
                } break;
                case NS_JSON_T_JSON: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore string JSON
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%s", p->key, (const char*)p->val);
                } break;
                case NS_JSON_T_OBJECT: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore object
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%s", p->key, ns_json_encode(mp, p->val, NS_JSON_T_OBJECT));
                } break;
                default: break;
              }
            }
          }
          if (arr->nelts > 0) {
            const char *tmp_s = ns_join(mp, arr, ",");
            if (tmp_s != NULL) {
              result = apr_psprintf(mp, "{%s}", tmp_s);
            }
          }
        }
      }
    }
  }
  return result;
}

/*
 * LOGGER
 */

ns_logger_t* ns_log_alloc(apr_pool_t *mp, apr_proc_mutex_t *m, const char *f,
                          apr_size_t sz)
{
  ns_logger_t *result = (ns_logger_t*)apr_palloc(mp, sizeof(ns_logger_t));
  if (result != NULL) {
    result->pool = mp;
    result->fname = f;
    result->mutex = m;
    result->max_size = sz ? sz : NS_LOG_MAX_FILE_SIZE;
    apr_status_t st = ns_file_open_append(&(result->fh), f, mp);
    if (st != APR_SUCCESS) {
      return NULL;
    }
    ns_log_rotate(result);
  }
  return result;
}

void ns_log_rotate(ns_logger_t *l)
{
  apr_finfo_t finfo;
  // Estraggo i metadati del file di log corrente
  apr_status_t rv = apr_file_info_get(&finfo, APR_FINFO_SIZE, l->fh);
  if (rv != APR_SUCCESS) {
    return;
  }
  // Estraggo la dimensione del file di log corrente
  apr_off_t sz = finfo.size;
  // Se la dimensione del file corrente è inferiore a quella massima termino
  if (sz < l->max_size) {
    return;
  }
  // Genero un nome di file per il file di log originale
  // con il timestamp unix corrente per non sovrascrivere file precedenti
  apr_time_t ts = ns_now();
  if (ts <= 0) {
    return;
  }
  const char *ts_s = apr_psprintf(l->pool, "%" APR_INT64_T_FMT, ts);
  if (ts_s == NULL) {
    return;
  }
  char *fname_old = apr_psprintf(l->pool, "%s_%s.old", l->fname, ts_s);
  if (fname_old == NULL) {
    return;
  }
  // Rinomino il file l->fname in fname_old
  // l->fh adesso punta al file fname_old pertanto le operazioni di
  // scrittura vengono registrate ancora sul file originale rinominato
  rv = apr_file_rename(l->fname, fname_old, l->pool);
  if (rv != APR_SUCCESS) {
    return;
  }
  // Apro un nuovo file con il nome l->fname
  // fh_new e l->fh non puntano allo stesso file
  apr_file_t *fh_new;
  rv = ns_file_open_truncate(&fh_new, l->fname, l->pool);
  if (rv != APR_SUCCESS) {
    // Provo a ripristinare il nome del file di ol originale
    apr_file_rename(fname_old, l->fname, l->pool);
    return;
  }
  // Scrivo '--log-rotate' sul file originale ancora puntato da l->fh
  int w_size = apr_file_printf(l->fh, "--log-rotate\r\n");
  if (w_size <= 0) {
    // Provo a ripristinare il nome del file di ol originale
    apr_file_rename(fname_old, l->fname, l->pool);
    return;
  }
  // Copio il descrittore di fh_new in l->fh
  // Da questo momento le oprazioni di scrittura usano sul nuovo file
  // l->fh e fh_new contengono 2 copie dello stesso descrittore
  rv = apr_file_dup2(l->fh, fh_new, l->pool);
  if (rv != APR_SUCCESS) {
    // Provo a ripristinare il nome del file di ol originale
    apr_file_rename(fname_old, l->fname, l->pool);
    return;
  }
  // Chiudo la copia del descrittore di file in fh_new
  apr_file_close(fh_new);
}

void ns_log_destroy(ns_logger_t *l)
{
  if (l != NULL) {
    if (l->fh != NULL) {
      apr_file_close(l->fh);
      l->fh = NULL;
    }
    l = NULL;
  }
}

/*
 * DBD
 */

ns_dbd_t* ns_dbd_alloc(apr_pool_t *mp)
{
  ns_dbd_t *result = NULL;
  if (mp != NULL) {
    if ((result = (ns_dbd_t*)apr_palloc(mp, sizeof(ns_dbd_t))) != NULL) {
      result->drv = NULL;
      result->hdl = NULL;
      result->er_msg = NULL;
      result->trx = NULL;
      result->err = 0;
    }
  }
  return result;
}

int ns_dbd_open(apr_pool_t *mp, ns_dbd_t *d, const char *s, const char *c)
{
  int result = 0;
  apr_status_t rv;
  d->er_msg = NULL;
  d->drv = NULL;
  d->hdl = NULL;
  d->err = 0;
  if (mp && d) {
    rv = apr_dbd_get_driver(mp, s, &(d->drv));
  }
  if (rv == APR_SUCCESS) {
    rv = apr_dbd_open_ex(d->drv, mp, c, &(d->hdl), &(d->er_msg));
  }
  result = rv == APR_SUCCESS;
  if (!result) {
    d->drv = NULL;
    d->hdl = NULL;
    d->err = 1;
  }
  return result;
}

const char *ns_dbd_escape(apr_pool_t *mp, ns_dbd_t *d, const char *s)
{
  return ((mp == NULL) || (d == NULL) || (s == NULL))
    ? NULL
    : apr_dbd_escape(d->drv, mp, s, d->hdl);
}

int ns_dbd_query(apr_pool_t *mp, ns_dbd_t *d, const char *sql)
{
  int result = 0;
  if (mp == NULL || d == NULL || sql == NULL) return -1;
  d->er_msg = NULL;
  d->err = apr_dbd_query(d->drv, d->hdl, &result, sql);
  if (d->err) {
    d->er_msg = apr_pstrdup(mp, apr_dbd_error(d->drv, d->hdl, d->err));
    return -1;
  }
  return result;
}

int ns_dbd_transaction_start(apr_pool_t *mp, ns_dbd_t *dbd)
{
  int rv = 1;
  const char *error;
  if ((mp != NULL) && (dbd != NULL)) {
    if ((rv = apr_dbd_transaction_start(dbd->drv, mp, dbd->hdl, &(dbd->trx)))) {
      if ((error = apr_dbd_error(dbd->drv, dbd->hdl, rv)) != NULL) {
        dbd->er_msg = apr_pstrdup(mp, error);
      }
    }
  }
  return (rv == 0 ? 0 : -1);
}

int ns_dbd_transaction_end(apr_pool_t *mp, ns_dbd_t *dbd)
{
  int rv = 1;
  const char *error;
  if ((mp != NULL) && (dbd != NULL)) {
    if ((rv = apr_dbd_transaction_end(dbd->drv, mp, dbd->trx))) {
      if ((error = apr_dbd_error(dbd->drv, dbd->hdl, rv)) != NULL) {
        dbd->er_msg = apr_pstrdup(mp, error);
      }
    }
  }
  return (rv == 0 ? 0 : -1);
}

apr_array_header_t* ns_dbd_result_to_array(apr_pool_t *mp, ns_dbd_t *dbd,
                                           apr_dbd_results_t *res)
{
  apr_table_t *rec;
  apr_dbd_row_t *row = NULL;
  apr_array_header_t *result = NULL;
  const char *k, *v;
  int rv, first_rec, num_fields;
  if ((mp != NULL) && (dbd != NULL) && (res != NULL)) {
    if ((rv = apr_dbd_get_row(dbd->drv, mp, res, &row, -1)) != -1) {
      first_rec = 1;
      while (rv != -1) {
        if (first_rec) {
          num_fields = apr_dbd_num_cols(dbd->drv, res);
          result = apr_array_make(mp, num_fields, sizeof(apr_table_t*));
          first_rec = 0;
        }
        rec = apr_table_make(mp, num_fields);
        for (int i = 0; i < num_fields; i++) {
          k = apr_dbd_get_name(dbd->drv, res, i);
          v = apr_dbd_get_entry(dbd->drv, row, i);
          apr_table_set(rec, apr_pstrdup(mp, k),
                        apr_pstrdup(mp, ns_is_empty(v) ? "NULL" : v));
        }
        APR_ARRAY_PUSH(result, apr_table_t*) = rec;
        rv = apr_dbd_get_row(dbd->drv, mp, res, &row, -1);
      }
    }
  }
  return result;
}

int ns_dbd_prepared_query(apr_pool_t *mp, ns_dbd_t *dbd, const char *sql,
                          apr_table_t *args)
{
  apr_table_entry_t *arg;
  const char **args_ar, *err;
  apr_dbd_prepared_t *stmt = NULL;
  int result = 0, nelts, rv;
  if (mp != NULL && dbd != NULL && sql != NULL) {
    dbd->er_msg = NULL;
    if ((nelts = apr_table_elts(args)->nelts) > 0) {
      args_ar = (const char**)apr_palloc(mp, sizeof(const char*)*nelts);
      if (args_ar != NULL) {
        for (int i = 0; i < nelts; i++) {
          arg = ns_table_entry(args, i);
          if (arg != NULL) {
            args_ar[i] = apr_pstrdup(mp, arg->val);
            if (args_ar[i] == NULL) {
              return -1;
            }
          }
        }
        dbd->err = apr_dbd_prepare(dbd->drv, mp, dbd->hdl, sql, NULL, &stmt);
        if (dbd->err) {
          err = apr_dbd_error(dbd->drv, dbd->hdl, dbd->err);
          dbd->er_msg = apr_pstrdup(mp, err);
          return -1;
        }
        dbd->err = apr_dbd_pquery(dbd->drv, mp, dbd->hdl, &result, stmt, nelts,
                                  args_ar);
        if (dbd->err) {
          err = apr_dbd_error(dbd->drv, dbd->hdl, dbd->err);
          dbd->er_msg = apr_psprintf(mp, "%s", err);
          return -1;
        }
      }
    }
  }
  return result;
}

// int ns_dbd_prepared_query(apr_pool_t *mp, ns_dbd_t *dbd,
//                           const char *sql, const char **args, int sz) {
  
//   const char *err;
//   apr_dbd_prepared_t *stmt = NULL;
//   int result = 0, rv;
//   if (mp != NULL && dbd != NULL && sql != NULL && args != NULL && sz > 0) {
//     dbd->er_msg = NULL;
//     rv = apr_dbd_prepare(dbd->drv, mp, dbd->hdl, sql, NULL, &stmt);
//     if (rv) {
//       err = apr_dbd_error(dbd->drv, dbd->hdl, rv);
//       dbd->er_msg = apr_pstrdup(mp, err);
//       return -1;
//     }
//     rv = apr_dbd_pquery(dbd->drv, mp, dbd->hdl, &result, stmt, sz, args);
//     if (rv) {
//       err = apr_dbd_error(dbd->drv, dbd->hdl, rv);
//       dbd->er_msg = apr_psprintf(mp, "%s", err);
//       return -1;
//     }
//   }
//   return result;
// }

apr_array_header_t* ns_dbd_prepared_select(apr_pool_t *mp, ns_dbd_t *dbd,
                                           const char *sql, apr_table_t *args)
{
  int rv, nelts;
  apr_dbd_results_t *res = NULL;
  apr_array_header_t *result = NULL;
  char **args_ar;
  const char *err;
  apr_table_entry_t *arg;
  apr_dbd_prepared_t *stmt = NULL;
  if ((mp != NULL) && (dbd != NULL) && (sql != NULL) && (args != NULL)) {
    if ((nelts = apr_table_elts(args)->nelts) > 0) {
      if ((args_ar = (char**)apr_palloc(mp, sizeof(char*)*nelts)) != NULL) {
        for (int i = 0; i < nelts; i++) {
          if ((arg = ns_table_entry(args, i)) != NULL) {
            if ((args_ar[i] = apr_psprintf(mp, "%s", arg->val)) == NULL) {
              return NULL;
            }
          }
        }
        rv = apr_dbd_prepare(dbd->drv, mp, dbd->hdl, sql, NULL, &stmt);
        if (rv) {
          err = apr_dbd_error(dbd->drv, dbd->hdl, rv);
          dbd->er_msg = apr_psprintf(mp, "%s", err);
          return NULL;
        }
        rv = apr_dbd_pselect(dbd->drv, mp, dbd->hdl, &res,
                             stmt, 0, nelts, (const char**)args_ar);
        if (rv) {
          err = apr_dbd_error(dbd->drv, dbd->hdl, rv);
          dbd->er_msg = apr_psprintf(mp, "%s", err);
          return NULL;
        }
        result = ns_dbd_result_to_array(mp, dbd, res);
      }
    }
  }
  return result;
}

apr_array_header_t* ns_dbd_select(apr_pool_t *mp, ns_dbd_t *d, const char *sql)
{
  int rv, err;
  apr_dbd_results_t *res = NULL;
  apr_dbd_row_t *row = NULL;
  apr_array_header_t *result = NULL;
  apr_table_t *rec;
  const char *k, *v;
  int first_rec, num_fields;
  if ((mp != NULL) && (d != NULL) && (sql != NULL)) {
    d->er_msg = NULL;
    if ((err = apr_dbd_select(d->drv, mp, d->hdl, &res, sql, 0))) {
      d->er_msg = apr_pstrdup(mp, apr_dbd_error(d->drv, d->hdl, err));
    } else {
      if (res != NULL) {
        if ((rv = apr_dbd_get_row(d->drv, mp, res, &row, -1)) != -1) {
          result = NULL;
          first_rec = 1;
          while (rv != -1) {
            if (first_rec) {
              num_fields = apr_dbd_num_cols(d->drv, res);
              result = apr_array_make(mp, num_fields, sizeof(apr_table_t*));
              first_rec = 0;
            }
            rec = apr_table_make(mp, num_fields);
            for (int i = 0; i < num_fields; i++) {
              k = apr_dbd_get_name(d->drv, res, i);
              v = apr_dbd_get_entry(d->drv, row, i);
              apr_table_set(rec, apr_pstrdup(mp, k),
                            apr_pstrdup(mp, ns_is_empty(v) ? "NULL" : v));
            }
            APR_ARRAY_PUSH(result, apr_table_t*) = rec;
            rv = apr_dbd_get_row(d->drv, mp, res, &row, -1);
          }
        }
      }
    }
  }
  return result;
}

int ns_dbd_num_records(apr_array_header_t *r) {
  return (int)(r != NULL ? r->nelts : 0);
}

int ns_dbd_num_columns(apr_array_header_t *r) {
  int result = 0;
  apr_table_t *rec;
  if (r && r->nelts) {
    if ((rec = APR_ARRAY_IDX(r, 0, apr_table_t*))) {
      result = apr_table_elts(rec)->nelts;
    }
  }
  return result;
}

apr_array_header_t* ns_dbd_column_names(apr_pool_t *mp, apr_array_header_t *r)
{
  int nelts;
  apr_table_entry_t* e;
  apr_table_t *rec;
  apr_array_header_t *result = NULL;
  if (r != NULL && r->nelts) {
    if ((rec = APR_ARRAY_IDX(r, 0, apr_table_t*))) {
      if ((nelts = (apr_table_elts(rec)->nelts))) {
        if ((result = apr_array_make(mp, nelts, sizeof(const char*)))) {
          for (int i = 0; i < nelts; i++) {
            if ((e = &((apr_table_entry_t*)((apr_table_elts(rec))->elts))[i])) {
              APR_ARRAY_PUSH(result, const char*) = apr_pstrdup(mp, e->key);
            }
          }
        }
      }
    }
  }
  return result;
}

apr_table_t* ns_dbd_record(apr_array_header_t *r, int i)
{
  return (r != NULL) && r->nelts && (i <= r->nelts-1)
    ? APR_ARRAY_IDX(r, i, apr_table_t*)
    : NULL;
}

const char *ns_dbd_field_value(apr_array_header_t *res, int i, const char *k)
{
  if (res == NULL || res->nelts <= 0 || i > (res->nelts-1)) return NULL;
  apr_table_t* rec = APR_ARRAY_IDX(res, i, apr_table_t*);
  return apr_table_get(rec, k);
}

int ns_dbd_field_set(apr_array_header_t *r, int i, const char *k, const char *v) {
  if (r == NULL || r->nelts <= 0 || i > (r->nelts-1)) return 1;
  apr_table_t* t = APR_ARRAY_IDX(r, i, apr_table_t*);
  apr_table_set(t, k, v);
  return 0;
}

int ns_dbd_close(ns_dbd_t *d) {
  return d == NULL ? 0 : apr_dbd_close(d->drv, d->hdl);
}

const char *ns_dbd_driver_name(ns_dbd_t *dbd) {
  return dbd == NULL ? NULL : apr_dbd_name(dbd->drv);
}

const char *ns_dbd_error(ns_dbd_t *d) {
  return (d == NULL) ? NULL : d->er_msg;
}

/*
 * HTTP REQUEST
 */

ns_http_request_t* ns_http_request_alloc(apr_pool_t *mp)
{
  ns_http_request_t *result = NULL;
  if (mp) {
    result = (ns_http_request_t*)apr_palloc(mp, sizeof(ns_http_request_t));
    if (result) {
      result->pool = mp;
      result->args = NULL;
      result->body = NULL;
      result->headers = apr_table_make(mp, 0);
      result->parsed_uri = apr_table_make(mp, 0);
      result->query = NULL;
      result->uri = NULL;
      result->message = NULL;
      result->multipart_data = NULL;
      result->cookies = apr_table_make(mp, 0);
      result->username = NULL;
      result->password = NULL;
    }
  }
  return result;
}

apr_table_t *ns_http_request_validate_args(ns_http_request_t *r,
                                           ns_request_validator_t *vd,
                                           int nargs) {
  int is_valid;
  const char *curr_v;
  apr_table_t *result = apr_table_make(r->pool, nargs);
  if (r && r->args) {
    for (int i = 0; i < nargs; ++i) {
      ns_request_validator_t v = vd[i];
      curr_v = apr_table_get(r->args, v.key);
      if (curr_v == NULL) {
        continue;
      }
      is_valid = 0;
      if (v.type == NS_REQUEST_T_INT) {
        is_valid = ns_is_int(curr_v);
      } else if (v.type == NS_REQUEST_T_DOUBLE) {
        is_valid = ns_is_double(curr_v);
      } else if (v.type == NS_REQUEST_T_STRING) {
        is_valid = !ns_is_empty(curr_v);
      } else if (v.type == NS_REQUEST_T_PASSWORD) {
        is_valid = !ns_is_empty(curr_v);
      } else if (v.type == NS_REQUEST_T_DATE) { // yyyy-mm-dd
        if (!ns_is_empty(curr_v) && strlen(curr_v) == 10) {
          apr_array_header_t *curr_v_ar = ns_split(r->pool, curr_v, "-");
          if (curr_v_ar && curr_v_ar->nelts == 3) {
            const char *y = APR_ARRAY_IDX(curr_v_ar, 0, const char*);
            const char *m = APR_ARRAY_IDX(curr_v_ar, 1, const char*);
            const char *d = APR_ARRAY_IDX(curr_v_ar, 2, const char*);
            is_valid = strlen(y) == 4 && ns_is_int(y) &&
                       strlen(m) == 2 && ns_is_int(m) &&
                       strlen(d) == 2 && ns_is_int(d);
          }
        }
      }
      if (is_valid) {
        if (v.filter == NS_REQUEST_F_MD5) {
          curr_v = ns_md5(r->pool, curr_v);
        }
        apr_table_set(result, v.key, curr_v);
      }
    }
  }
  return result;
}

apr_table_t* ns_http_request_validate_multipart_args(ns_http_request_t *r,
                                                     ns_request_validator_t *vd,
                                                     int nargs) {
  apr_table_t *result = NULL;
  if (r && vd && nargs) {
    int is_valid;
    const char *req_v;
    result = apr_table_make(r->pool, nargs);
    for (int i = 0; i < nargs; ++i) {
      ns_request_validator_t v = vd[i];
      for (int j = 0; j < r->multipart_data->nelts; ++j) {
        apr_table_t *entry = APR_ARRAY_IDX(r->multipart_data, j, apr_table_t*);
        if (!entry) {
          continue;
        }
        const char *key = apr_table_get(entry, "name");
        if (!key || (strcmp(v.key, key) != 0)) {
          continue;
        }
        req_v = apr_table_get(entry, "value");
        if (!req_v) {
          continue;
        }
        is_valid = 0;
        if (v.type == NS_REQUEST_T_INT) {
          is_valid = ns_is_int(req_v);
        } else if (v.type == NS_REQUEST_T_DOUBLE) {
          is_valid = ns_is_double(req_v);
        } else if (v.type == NS_REQUEST_T_STRING) {
          is_valid = !ns_is_empty(req_v);
        } else if (v.type == NS_REQUEST_T_PASSWORD) {
          is_valid = !ns_is_empty(req_v);
        } else if (v.type == NS_REQUEST_T_DATE) { // yyyy-mm-dd
          if (!ns_is_empty(req_v) && strlen(req_v) == 10) {
            apr_array_header_t *req_v_ar = ns_split(r->pool, req_v, "-");
            if (req_v_ar && req_v_ar->nelts == 3) {
              const char *y = APR_ARRAY_IDX(req_v_ar, 0, const char*);
              const char *m = APR_ARRAY_IDX(req_v_ar, 1, const char*);
              const char *d = APR_ARRAY_IDX(req_v_ar, 2, const char*);
              is_valid = strlen(y) == 4 && ns_is_int(y) &&
                         strlen(m) == 2 && ns_is_int(m) &&
                         strlen(d) == 2 && ns_is_int(d);
            }
          }
        }
        if (is_valid) {
          if (v.filter == NS_REQUEST_F_MD5) {
            req_v = ns_md5(r->pool, req_v);
          }
          apr_table_set(result, v.key, req_v);
        }
      }
    }
  }
  return result;
}

/*
 * HTTP RESPONSE
 */

ns_http_response_t* ns_http_response_alloc(apr_pool_t *mp) {
  ns_http_response_t *result = NULL;
  if (mp) {
    result = (ns_http_response_t*)apr_palloc(mp, sizeof(ns_http_response_t));
  }
  if (result) {
    result->pool = mp;
    result->headers = apr_table_make(mp, 0);
    result->status = 0;
    result->size = 0;
    result->buffer = NULL;
  }
  return result;
}

void ns_http_response_hd_set(ns_http_response_t *r, const char *k, const char *v) {
  if (r && k && v) {
    apr_table_set(r->headers, k, v);
  }
}

const char *ns_http_response_hd_get(ns_http_response_t *r, const char *k) {
  return r && k ? apr_table_get(r->headers, k) : NULL;
}

const char *ns_http_response_hd_serialize(ns_http_response_t *r) {
  const char *result = NULL;
  do {
    if (!r) break;
    int nelts = ns_table_nelts(r->headers);
    if (nelts <= 0) break;
    apr_table_entry_t *e;
    e = ns_table_entry(r->headers, 0);
    result = apr_psprintf(r->pool, "%s: %s\r\n", e->key, e->val);
    for (int i = 1; i < nelts; i++) {
      e = ns_table_entry(r->headers, i);
      if (e) {
        char *h = apr_psprintf(r->pool, "%s: %s\r\n", e->key, e->val);
        result = apr_pstrcat(r->pool, result, h, NULL);
      }
    }
  } while(0);
  return result;
}

void ns_http_response_buffer_set(ns_http_response_t *r, void *buf, size_t sz) {
  if (r && buf && sz) {
    r->size = sz;
    r->buffer = apr_palloc(r->pool, sz);
    if (r->buffer) {
      memcpy(r->buffer, buf, sz);
    }
  }
}

/*
 * SERVICE
 */

ns_service_t* ns_alloc(apr_pool_t *mp) {
  ns_service_t *result = NULL;
  if (mp) {
    result = (ns_service_t*)apr_palloc(mp, sizeof(ns_service_t));
  }
  if (result) {
    result->pool = mp;
    result->authorized = 0;
    result->er_msg = NULL;
    result->request = NULL;
    result->response = NULL;
    result->dbd = NULL;
    result->logger = NULL;
  }
  return result;
}

void ns_route(ns_service_t *s, const char *mth, const char *uri, ns_route_t fn) {
  if (s && mth && uri && fn) {
    if (s->response && !s->response->status) {
      if (s->request) {
        if (s->request->method && strcmp(s->request->method, mth) == 0) {
          if (s->request->uri && strcmp(s->request->uri, uri) == 0) {
            s->response->status = fn(s);
          }
        }
      }
    }
  }
}

void ns_printf(ns_service_t *s, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  // Calcolo la lunghezza della stringa formattata
  apr_size_t len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  if (len > 0) {
    // Allocazione la memoria per la stringa formattata
    char *buffer = (char*)apr_palloc(s->pool, len + 1);
    if (buffer) {
      // Creo la stringa formattata
      va_start(args, fmt);
      vsnprintf(buffer, len + 1, fmt, args);
      va_end(args);
      buffer[len+1] = '\0';
      s->response->is_binary = 0;
      if (s->response->buffer == NULL) {
        // Inizializzo il body della response HTTP
        s->response->buffer = apr_pstrdup(s->pool, buffer);
      } else {
        // Concateno la stringa al body della response HTTP
        s->response->buffer = apr_pstrcat(s->pool, s->response->buffer, buffer, NULL);
      }
      if (s->response->buffer) {
        s->response->size = strlen(s->response->buffer);
      }
    }
  }
}

char *ns_jwt_base64_encode(const unsigned char *s, int sz) {
  char *result = NULL;
  if (s && sz) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, s, sz);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);
    result = bufferPtr->data;
  }
  return result;
}

unsigned char *ns_jwt_base64_decode(const char *s, int sz) {
  unsigned char *result = NULL;
  if (s && sz) {
    BIO *bio, *b64;
    result = (unsigned char*)malloc(sz);
    int decode_length = 0;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(s, sz);
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    decode_length = BIO_read(bio, result, sz);
    BIO_free_all(bio);
    // Trim padding characters '='
    while (result[decode_length - 1] == '=') {
      decode_length--;
    }
    result = realloc(result, decode_length + 1);
    result[decode_length] = '\0';
  }
  return result;
}

char *ns_hmac_encode(const char *key, const char *s, apr_size_t sz) {
  char *result = NULL;
  if (key && s && sz) {
    unsigned int hmac_len;
    unsigned char hmac[EVP_MAX_MD_SIZE];
    HMAC(EVP_sha256(), key, strlen(key), (const unsigned char*)s, sz, hmac, &hmac_len);
    HMAC(EVP_sha256(), key, strlen(key), (const unsigned char*)s, sz, hmac, &hmac_len);
    result = ns_jwt_base64_encode(hmac, hmac_len);
  }
  return result;
}

char *ns_jwt_token_create(apr_pool_t *mp, apr_table_t *claims, const char *key) {
  char *result = NULL;
  const char *claims_str = NULL;
  char *enc_head = NULL, *enc_hmac = NULL, *enc_claims = NULL;
  const unsigned char head[] = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
  if (mp && claims && key) {
    claims_str = ns_json_encode(mp, claims, NS_JSON_T_TABLE);
  }
  if (claims_str) {
    enc_head = ns_jwt_base64_encode(head, 27);
  }
  if (enc_head) {
    enc_claims = ns_jwt_base64_encode((const unsigned char*)claims_str, strlen(claims_str));
  }
  if (enc_claims) {
    enc_hmac = ns_hmac_encode(key, enc_claims, strlen(enc_claims));
  }
  if (enc_hmac) {
    result = apr_psprintf(mp, "%s.%s.%s", enc_head, enc_claims, enc_hmac);
    free(enc_hmac);
  }
  return result;
}

int ns_jwt_token_validate(apr_pool_t *mp, const char *tok, const char *key) {
  int result = 0;
  const char *enc_claims = NULL, *enc_hmac = NULL, *gen_hmac = NULL;
  apr_array_header_t *tok_ar;
  if (mp && tok && key) {
    tok_ar = ns_split(mp, tok, ".");
  }
  if (tok_ar && tok_ar->nelts == 3) {
    enc_claims = APR_ARRAY_IDX(tok_ar, 1, const char*);
  }
  if (enc_claims) {
    enc_hmac = APR_ARRAY_IDX(tok_ar, 2, const char*);
  }
  if (enc_hmac) {
    gen_hmac = (const char*)ns_hmac_encode(key, enc_claims, strlen(enc_claims));
  }
  if (gen_hmac) {
    result = (int)(strcmp(enc_hmac, gen_hmac) == 0);
  }
  return result;
}


// -----------------------------------------------------------------------------

ns_server_t* ns_server_alloc(apr_pool_t *mp) {
  ns_server_t *s;
  s = (ns_server_t*)apr_palloc(mp, sizeof(ns_server_t));
  if (s != NULL) {
    s->pool = mp;
    s->host = NULL;
    s->port = NULL;
    s->port_s = NULL;
    s->timeout = NULL;
    s->max_threads = NULL;
    s->log_file = NULL;
    s->logger = NULL;
    s->dbd_driver = NULL;
    s->dbd_conn_s = NULL;
    s->upload_dir = NULL;
    s->addr = NULL;
    s->addr_s = NULL;
  }
  return s;
}

void ns_server_destroy(ns_server_t *s) {
  if (s && s->pool) {
    if (s->logger) {
      if (s->logger->mutex) {
        apr_proc_mutex_destroy(s->logger->mutex);
      }
      if (s->logger->fh) {
        apr_file_close(s->logger->fh);
      }
    }
  }
}

int ns_dbd_pool_alloc(apr_pool_t *mp) {
  dbd_pool = (ns_dbd_pool_t*)apr_palloc(mp, sizeof(ns_dbd_pool_t));
  return dbd_pool != NULL;
}

void ns_dbd_pool_add(apr_pool_t *mp, const char *drv, const char *conn_s) {
  ns_dbd_t *dbd = ns_dbd_alloc(mp);
  if (dbd != NULL) {
    if (ns_dbd_open(mp, dbd, drv, conn_s)) {
      APR_ARRAY_PUSH(dbd_pool->connections, ns_dbd_t*) = dbd;
      dbd_pool->counter ++;
    }
  }
}

int ns_dbd_pool_init(apr_pool_t *mp, const char *drv, const char *conn_s) {
  int result = 0;
  if (dbd_pool != NULL) {
    dbd_pool->counter = -1;
    dbd_pool->connections = apr_array_make(
      mp, NS_DBD_POOL_INIT_SIZE, sizeof(ns_dbd_t*));
    if (dbd_pool->connections != NULL) {
      apr_status_t rv;
      rv = apr_proc_mutex_create(
        &(dbd_pool->mutex), "dbd_pool_mutex", APR_LOCK_DEFAULT, mp);
      if (rv == APR_SUCCESS) {
        for (int i = 0; i < NS_DBD_POOL_INIT_SIZE; ++i) {
          ns_dbd_pool_add(mp, drv, conn_s);
        }
        result = dbd_pool->connections->nelts == NS_DBD_POOL_INIT_SIZE;
      }
    }
  }
  return result;
}

ns_dbd_t* ns_dbd_pool_get() {
  ns_dbd_t *dbd = NULL;
  apr_status_t rv = apr_proc_mutex_lock(dbd_pool->mutex);
  if (rv == APR_SUCCESS) {
    if (dbd_pool->connections->nelts > 0) {
      if (dbd_pool->counter >= 0) {
        dbd = APR_ARRAY_IDX(dbd_pool->connections, dbd_pool->counter, ns_dbd_t*);
        dbd_pool->counter --;
      }
    }
    apr_proc_mutex_unlock(dbd_pool->mutex);
  }
  return dbd;
}

void ns_dbd_pool_release() {
  if (dbd_pool != NULL && dbd_pool->connections != NULL) {
    apr_status_t rv = apr_proc_mutex_lock(dbd_pool->mutex);
    if (rv == APR_SUCCESS) {
      if (dbd_pool->connections->nelts > 0) {
        if (dbd_pool->counter < NS_DBD_POOL_INIT_SIZE - 1) {
          dbd_pool->counter ++;
        }
      }
      apr_proc_mutex_unlock(dbd_pool->mutex);
    }
  }
}

void ns_dbd_pool_destroy() {
  if (dbd_pool != NULL) {
    apr_status_t rv = apr_proc_mutex_lock(dbd_pool->mutex);
    if (rv == APR_SUCCESS) {
      if (dbd_pool->connections != NULL && dbd_pool->connections->nelts > 0) {
        for (int i = 0; i < NS_DBD_POOL_INIT_SIZE; ++i) {
          ns_dbd_t *dbd = APR_ARRAY_IDX(dbd_pool->connections, i, ns_dbd_t*);
          if (dbd != NULL && dbd->drv && dbd->hdl) {
            ns_dbd_close(dbd);
          }
        }
      }
      apr_proc_mutex_unlock(dbd_pool->mutex);
    }
  }
}

void ns_http_request_headers_set(apr_pool_t *mp, ns_http_request_t *rq, struct mg_http_message *hm) {
  apr_table_t *headers = NULL;
  size_t i, max = sizeof(hm->headers) / sizeof(hm->headers[0]);
  for (i = 0; i < max && hm->headers[i].name.len > 0; i++) {
    if (!(rq->headers)) {
      rq->headers = apr_table_make(mp, 1);
      if (!(rq->headers)) {
        break;
      }
    }
    struct mg_str *k, *v;
    const char *key = NULL, *val = NULL;
    k = &hm->headers[i].name;
    if (k) {
      key = apr_psprintf(mp, "%.*s", (int) k->len, k->ptr);  
    }
    v = &hm->headers[i].value;
    if (v) {
      val = apr_psprintf(mp, "%.*s", (int) v->len, v->ptr);
    }
    if (key && val) {
      apr_table_set(rq->headers, key, val);
    }
  }
}

apr_table_t* ns_http_request_args_parse(apr_pool_t *mp, const char *s, const char *sp1, const char *sp2) {
  apr_table_t *result = NULL;
  if (mp && s && sp1 && sp2) {
    apr_array_header_t *ar = ns_split(mp, s, sp1);
    if (ar && ar->nelts > 0) {
      for (int i = 0; i < ar->nelts; i++ ) {
        const char *entry = APR_ARRAY_IDX(ar, i, const char*);
        apr_array_header_t *pair = ns_split(mp, entry, sp2);
        if (pair && pair->nelts > 0) {
          if (!result) {
            result = apr_table_make(mp, 1);
            if (!result) {
              return NULL;
            }
          }
          char *k, *v;
          char *trm_k, *trm_v;
          k = apr_pstrdup(mp, APR_ARRAY_IDX(pair, 0, const char*));
          v = apr_pstrdup(mp, APR_ARRAY_IDX(pair, 1, const char*));
          trm_k = ns_trim(mp, k);
          trm_v = ns_trim(mp, v);
          apr_table_set(result, trm_k, trm_v);
        }
      }
    }
  }
  return result;
}

apr_table_t* ns_http_request_cookies_parse(apr_pool_t *mp, struct mg_http_message *hm) {
  apr_table_t *result = NULL;
  struct mg_str *cookies = mg_http_get_header(hm, "Cookie");
  if (cookies != NULL) {
    result = ns_http_request_args_parse(mp, cookies->ptr, ";", "=");
  }
  return result;
}

apr_table_t* ns_http_query_string_parse(apr_pool_t*mp, struct mg_http_message *hm) {
  apr_table_t *result = NULL;
  if (hm->query.len > 0) {
    const char *query = ns_str(mp, hm->query.ptr, hm->query.len);
    result = ns_http_request_args_parse(mp, query, "&", "=");
  }
  return result;
}

apr_table_t* ns_http_request_body_parse(apr_pool_t*mp, struct mg_http_message *hm) {
  apr_table_t *result = NULL;
  if (hm->body.len > 0) {
    const char *body = ns_str(mp, hm->body.ptr, hm->body.len);
    result = ns_http_request_args_parse(mp, body, "&", "=");
  }
  return result;
}

// void ns_signal_exit(int signum) {
//   if (signum == SIGTERM || signum == SIGINT) {
//     server_run = 0;
//   }
// }

void ns_signal_handler(struct sigaction *sig_action, sighd_t cb) {
  sig_action->sa_handler = cb;
  sigemptyset(&sig_action->sa_mask);
  sig_action->sa_flags = 0;
  sigaction(SIGTERM, sig_action, NULL);
  sigaction(SIGINT, sig_action, NULL);
}

int ns_http_request_multipart_parse(apr_pool_t *mp, ns_http_request_t *rq, struct mg_connection *c, struct mg_http_message *hm) {
  char *err;
  apr_size_t fsize;
  const char *fname, *forig, *fpath;
  apr_table_t *entry;
  apr_file_t *fd;
  apr_status_t rv;
  struct mg_http_part part;
  size_t ofs = 0;
  rq->multipart_data = apr_array_make(mp, 0, sizeof(apr_table_t*));
  while ((ofs = mg_http_next_multipart(hm->body, ofs, &part)) != 0) {
    entry = apr_table_make(mp, 2);
    // Multipart files
    if ((int)part.filename.len > 0) {
      fname = apr_psprintf(mp, "%.*s", (int)part.name.len, part.name.ptr);
      forig = apr_psprintf(
        mp, "%.*s", (int)part.filename.len, part.filename.ptr);
      fsize = (apr_size_t)part.body.len;
      fpath = apr_psprintf(mp, "%s/%s", "/tmp", forig);
      rv = ns_file_open_truncate(&fd, fpath, mp);
      if (rv == APR_SUCCESS) {
        fsize = ns_file_write(fd, part.body.ptr, fsize);
        apr_table_set(entry, "file_name", fname);
        apr_table_set(entry, "file_path", fpath);
      }
    }
    // Multipart arguments
    else {
      char *key = apr_psprintf(mp, "%.*s", (int)part.name.len, part.name.ptr);
      apr_table_set(entry, "name", key);
      char *val = apr_psprintf(mp, "%.*s", (int)part.body.len, part.body.ptr);
      apr_table_set(entry, "value", val);
    }
    APR_ARRAY_PUSH(rq->multipart_data, apr_table_t*) = entry;
  }
  return rq->multipart_data->nelts > 0;
}

void ns_http_request_handler(struct mg_connection *c, int ev, void *ev_data) {
  struct state_t {
    struct flag_t {
      int ev_data, fn_data, init, pool, logger, request, method, uri, query,
          body, multipart, response, handler, resp_headers, resp_size, dbd,
          dbd_handler, service;
    } flag;
    int error;
    apr_pool_t *pool;
    ns_server_t *server;
    struct mg_http_message *hm;
    const char *er_msg;
  } st = {
    .flag.ev_data = 0, .flag.fn_data = 0, .flag.init = 0, .flag.pool = 0,
    .flag.logger = 0, .flag.request = 0, .flag.method = 0, .flag.uri = 0,
    .flag.query = 0, .flag.body = 0, .flag.multipart = 0, .flag.response = 0,
    .flag.resp_headers = 0, .flag.handler = 0, .flag.dbd = 0,
    .flag.dbd_handler = 0, .flag.service = 0, .flag.resp_size = 0, .error = 0,
    .server = NULL, .er_msg = NULL
  };
  do {
    if (ev == MG_EV_ACCEPT && c->fn_data != NULL) {
      #ifdef _TLS
      struct mg_tls_opts opts = {
        #ifdef _TLS_TWOWAY
        .ca = mg_str(s_tls_ca),
        #endif
        .cert = mg_str(s_tls_cert),
        .key = mg_str(s_tls_key)
      };
      mg_tls_init(c, &opts);
      #endif
    }
    if (ev == MG_EV_HTTP_MSG) {
      st.flag.ev_data = ev_data != NULL;
      if ((st.error = !st.flag.ev_data)) break;
      // Event data
      st.hm = (struct mg_http_message*)ev_data;
      st.flag.fn_data = c->fn_data != NULL;
      if ((st.error = !st.flag.fn_data)) break;
      // Server data
      st.server = (ns_server_t*)c->fn_data;
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "Client connected");
      }
      {
        apr_status_t rv;
        // APR initialization
        rv = apr_initialize();
        st.flag.init = (rv == APR_SUCCESS);
        if ((st.error = !st.flag.init)) break;
        if (DEBUG) {
          ns_log((st.server)->logger, "INFO", "Service APR initialized");
        }
        // Memory pool allocation
        rv = apr_pool_create(&st.pool, NULL);
        st.flag.pool = (rv == APR_SUCCESS);
        if ((st.error = !st.flag.pool)) break;
        if (DEBUG) {
          ns_log((st.server)->logger, "INFO", "Service pool created");
        }
      }
      // Service allocation
      ns_service_t* sv = ns_alloc(st.pool);
      st.flag.service = sv != NULL;
      if ((st.error = !st.flag.service)) break;
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "Service data struct allocated");
      }
      // Logger
      sv->logger = (st.server)->logger;
      st.flag.logger = sv->logger != NULL;
      if ((st.error = !st.flag.logger)) break;
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "Service logger initialized");
      }
      // Request
      sv->request = ns_http_request_alloc(st.pool);
      st.flag.request = sv->request != NULL;
      if ((st.error = !st.flag.request)) break;
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "Service HTTP request allocated");
      }
      // Request headers
      ns_http_request_headers_set(st.pool, sv->request, st.hm);
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "HTTP request headers parsed");
      }
      // Request method
      st.flag.method = (st.hm)->method.len > 0;
      if ((st.error = !st.flag.method)) break;
      sv->request->method = ns_str(
        st.pool, (st.hm)->method.ptr, (st.hm)->method.len);
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "HTTP request method parsed");
      }
      // Request URI
      st.flag.uri = (st.hm)->uri.len > 0;
      if ((st.error = !st.flag.uri)) break;
      sv->request->uri = ns_str(st.pool, (st.hm)->uri.ptr, (st.hm)->uri.len);
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "HTTP request uri parsed");
      }
      // Request query string
      if (strcmp(sv->request->method, "GET") == 0) {
        if ((st.hm)->query.len) {
          sv->request->query = ns_str(
            st.pool, (st.hm)->query.ptr, (st.hm)->query.len);
          st.flag.query = sv->request->query != NULL;
          if ((st.error = !st.flag.query)) break;
          sv->request->args = ns_http_query_string_parse(sv->pool, st.hm);
          if (sv->request->args) {
            if (DEBUG) {
              ns_log((st.server)->logger, "INFO", "HTTP query string parsed");
            }
          }
        } else {
          st.flag.query = 1;
        }
      }
      // Request body
      if ((st.hm)->body.len) {
        sv->request->body = ns_str(st.pool, (st.hm)->body.ptr, (st.hm)->body.len);
        st.flag.body = sv->request->body != NULL;
        if ((st.error = !st.flag.body)) break;
        sv->request->args = ns_http_request_body_parse(sv->pool, st.hm);
        if (sv->request->args) {
          if (DEBUG) {
            ns_log((st.server)->logger, "INFO", "HTTP body parsed");
          }
        }
      } else {
        st.flag.body = 1;
      }
      {
        // Request multipart data
        const char *ctype;
        ctype = apr_table_get(sv->request->headers, "Content-Type");
        if (ns_in_string(ctype, "multipart/form-data")) {
          st.flag.multipart =
            ns_http_request_multipart_parse(st.pool, sv->request, c, st.hm);
          if ((st.error = !st.flag.multipart)) break;
          if (DEBUG) {
            ns_log((st.server)->logger, "INFO", "HTTP request multipart parsed");
          }
        } else {
          st.flag.multipart = 1;
        }
      }
      // Authorization
      {
        char user[256] = {0}, pass[256] = {0};
        mg_http_creds(st.hm, user, sizeof(user), pass, sizeof(pass));
        if (strlen(user) > 0) {
          sv->request->username = apr_pstrdup(st.pool, user);
        }
        if (strlen(pass) > 0) {
          sv->request->password = apr_pstrdup(st.pool, pass);
        }
      }
      // Response
      sv->response = ns_http_response_alloc(st.pool);
      st.flag.response = sv->response != NULL;
      if ((st.error = !st.flag.response)) break;
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "HTTP response allocated");
      }
      // Default response HTTP header Content-Type
      ns_http_response_hd_set(sv->response, "Content-Type", "text/plain");
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "HTTP response Content-Type defined");
      }
      // DBD connection
      if ((st.server)->dbd_driver && (st.server)->dbd_conn_s) {
        if (DEBUG) {
          ns_log((st.server)->logger, "INFO", "DBD connection configured");
        }
        sv->dbd = ns_dbd_pool_get();
        st.flag.dbd = sv->dbd != NULL;
        if ((st.error = !st.flag.dbd)) break;
        if (DEBUG) {
          ns_log((st.server)->logger, "INFO", "DBD connection opened");
        }
        st.flag.dbd_handler = sv->dbd->drv != NULL && sv->dbd->hdl != NULL;
        if ((st.error = !st.flag.dbd_handler)) break;
        if (DEBUG) {
          ns_log((st.server)->logger, "INFO", "DBD handler initialized.");
        }
      }
      // ---------------
      // Service handler
      // ---------------
      ns_handler(sv);
      st.er_msg = sv->er_msg;
      st.flag.handler = st.er_msg == NULL;
      if ((st.error = !st.flag.handler)) break;
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "Service handler executed.");
      }
      ns_dbd_pool_release();
      if (DEBUG) {
        ns_log((st.server)->logger, "INFO", "DBD connection released.");
      }
      // Response headers
      const char *http_hd;
      http_hd = ns_http_response_hd_serialize(sv->response);
      st.flag.resp_headers = http_hd != NULL;
      if ((st.error = !st.flag.resp_headers)) break;
      // Default HTTP response status (404 - Not Found)
      if (sv->response->status == 0) {
        sv->response->status = 404;
      }
      if (sv->response->status != 200) {
        // Not 200 OK response
        const char *ctype;
        ctype = ns_http_response_hd_get(sv->response, "Content-Type");
        if (ctype) {
          ctype = apr_psprintf(st.pool, "Content-Type: %s\r\n", ctype);
        }
        mg_http_reply(c, sv->response->status, ctype, "");
      } else {
        // 200 OK empty response
        st.flag.resp_size = sv->response->size > 0;
        if ((st.error = !st.flag.resp_size)) break;
        // 200 OK response
        mg_printf(c, HTTP_OK_FMT, http_hd ? http_hd : "", sv->response->size);
        if ((sv->response->size > 0) && sv->response->buffer) {
          mg_send(c, sv->response->buffer, sv->response->size);
        }
      }
    } /* ev == MG_EV_HTTP_MSG */
  } while (0);
  if (st.error) {
    const char ctype[] = "Content-Type: text/plain\r\n";
    if (!st.flag.ev_data) {
      mg_http_reply(c, 500, ctype, "Invalid event data.\r\n");
    } else if (!st.flag.fn_data) {
      mg_http_reply(c, 500, ctype, "Invalid server data.\r\n");
    } else if (!st.flag.init) {
      mg_http_reply(c, 500, ctype, "APR initialization error.\r\n");
    } else if (!st.flag.pool) {
      mg_http_reply(c, 500, ctype, "APR memory pool error.\r\n");
    } else if (!st.flag.service) {
      if (st.er_msg) {
        mg_http_reply(c, 500, ctype, st.er_msg);
      } else {
        mg_http_reply(c, 500, ctype, "HTTP service error.\r\n");
      }
    } else if (!st.flag.logger) {
      mg_http_reply(c, 500, ctype, "Invalid logger.\r\n");
    } else if (!st.flag.request) {
      mg_http_reply(c, 500, ctype, "Invalid request allocation.\r\n");
    } else if (!st.flag.method) {
      mg_http_reply(c, 500, ctype, "Invalid request method.\r\n");
    } else if (!st.flag.uri) {
      mg_http_reply(c, 500, ctype, "Invalid request uri.\r\n");
    } else if (!st.flag.query) {
      mg_http_reply(c, 500, ctype, "Invalid request query string.\r\n");
    } else if (!st.flag.body) {
      mg_http_reply(c, 500, ctype, "Invalid request body.\r\n");
    } else if (!st.flag.multipart) {
      mg_http_reply(c, 500, ctype, "Invalid request multipart data.\r\n");
    } else if (!st.flag.response) {
      mg_http_reply(c, 500, ctype, "Response allocation failure.\r\n");
    } else if (!st.flag.dbd) {
      mg_http_reply(c, 500, ctype, "DBD error.\r\n");
    } else if (!st.flag.dbd_handler) {
      mg_http_reply(c, 500, ctype, "DBD connection error.\r\n");
    } else if (!st.flag.handler) {
      if (st.er_msg) {
        mg_http_reply(c, 500, ctype, st.er_msg);
      } else {
        mg_http_reply(c, 500, ctype, "Request handler error.\r\n");
      }
    } else if (!st.flag.resp_headers) {
      mg_http_reply(c, 500, ctype, "Invalid HTTP response headers.\r\n");
    } else if (!st.flag.resp_size) {
      mg_http_reply(c, 500, ctype, "Empty response.\r\n");
    } else {
      mg_http_reply(c, 500, ctype, "General error.\r\n");
    }
  }
  if (st.flag.init) {
    if (st.flag.pool) {
      apr_pool_destroy(st.pool);
    }
    apr_terminate();
  }
}

int ns_cmd_args_parse(ns_server_t *s, int argc, char *argv[], char **er_msg) {
  struct state_t {
    struct flag_t {
      int input, arg_format, host, port, log_file;
    } flag;
    int error, result;
  } st = {
    .flag.input = 0, .flag.arg_format = 0, .error = 0, .result = 0
  };
  do {
    *er_msg = NULL;
    // Input validation
    st.flag.input = s != NULL && argv != NULL && argc > 1 && ((argc-1)%2) == 0;
    if ((st.error = !st.flag.input)) break;
    for (int i = 1; i < argc; i += 2) {
      // Command line arguments validation
      st.flag.arg_format = strlen(argv[i]) == 2;
      if ((st.error = !st.flag.arg_format)) break;
      // Command line arguments value
      if (argv[i][1] == 'h') {
        s->host = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'p') {
        s->port = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'P') {
        // https port
        s->port_s = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 't') {
        s->timeout = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'T') {
        s->max_threads = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'l') {
        s->log_file = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'd') {
        s->dbd_driver = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'D') {
        s->dbd_conn_s = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'u') {
        s->upload_dir = apr_pstrdup(s->pool, argv[i+1]);
      }
    }
    st.error = st.error || !(s->host) || !(s->port) || !(s->log_file);
    if (st.error) break;
    st.result = 1;
  } while (0);

  if (st.error) {
    if (!st.flag.input) {
      *er_msg = apr_pstrdup(s->pool, "Invalid input.");
    } else if (!st.flag.arg_format) {
      *er_msg = apr_pstrdup(s->pool, "Invalid arguments format.");
    } else if (s->host == NULL) {
      *er_msg = apr_pstrdup(s->pool, "Invalid host address.");
    } else if (s->port == NULL) {
      *er_msg = apr_pstrdup(s->pool, "Invalid port number.");
    } else if (s->log_file == NULL) {
      *er_msg = apr_pstrdup(s->pool, "Invalid log file.");
    } else {
      *er_msg = apr_pstrdup(s->pool, "General error.");
    }
  }
  return st.result;
}

int ns_server_init(apr_pool_t *mp, ns_server_t **s, int argc, char *argv[], char **er_msg) {
  struct state_t {
    struct flag_t {
      int input, args, addr, mutex, logger;
    } flag;
    int error, result;
    apr_status_t mutex;
    apr_proc_mutex_t *log_mutex;
  } st = {
    .flag.input = 0, .flag.args = 0, .flag.addr = 0, .flag.mutex = 0,
    .flag.logger = 0, .error = 0, .result = 0
  };
  do {
    *er_msg = NULL;
    st.flag.input = mp != NULL && *s != NULL && argv != NULL && argc > 1;
    if ((st.error = !st.flag.input)) break;
    st.flag.args = ns_cmd_args_parse(*s, argc, argv, er_msg);
    if ((st.error = !st.flag.args)) break;
    (*s)->addr = apr_psprintf(mp, "%s:%s", (*s)->host, (*s)->port);
    if (TLS) {
      if ((*s)->port_s) {
        (*s)->addr_s = apr_psprintf(mp, "%s:%s", (*s)->host, (*s)->port_s);
      }
    }
    st.flag.addr = (*s)->addr != NULL;
    if ((st.error = !st.flag.addr)) break;
    apr_status_t rv;
    rv = apr_proc_mutex_create(&(st.log_mutex), "log_mutex", APR_LOCK_DEFAULT, mp);
    st.flag.mutex = rv == APR_SUCCESS;
    if ((st.error = !st.flag.mutex)) break;
    (*s)->logger = ns_log_alloc(mp, st.log_mutex, (*s)->log_file, 0);
    st.flag.logger = (*s)->logger != NULL;
    if ((st.error = !st.flag.logger)) break;
    st.result = 1;
  } while (0);
  if (st.error) {
    if (!st.flag.input) {
      *er_msg = apr_pstrdup(mp, "Invalid input");
    } else if (!st.flag.args) {
      if (*er_msg == NULL) {
        *er_msg = apr_pstrdup(mp, "Invalid arguments");
      }
    } else if (!st.flag.args) {
      *er_msg = apr_pstrdup(mp, "Invalid address");
    } else if (!st.flag.mutex) {
      *er_msg = apr_pstrdup(mp, "Logger mutex initialization error");
    } else if (!st.flag.logger) {
      *er_msg = apr_pstrdup(mp, "Logger initialization error");
    } else {
      *er_msg = apr_pstrdup(mp, "General error");
    }
  }
  return st.result;
}
