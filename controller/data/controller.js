axios.defaults.headers.common['Content-Type'] =
  'application/x-www-form-urlencoded';

var app = new Vue({
  el: '#app',
  data() {
    return {
      message: 'Good Morning!',
      apptitle: 'WiFi Controlled Strip Lighting',
      showsecondary: true,
      mix: 'single',
      effect: 'solid',
      colorone: 'FFFF00',
      colortwo: '0000FF',
      hasError: false,
      rate: 10,
    };
  },
  filters: {
    filterRate: function(value) {
      return `${(value / 1000).toFixed(2)} s`;
    }
  },
  computed: {},
  // list of methods related to this vue instance
  methods: {
    getColour() {
      return (
        document.getElementById('primary').textContent ||
        document.getElementById('primary').value ||
        'check Code'
      );
    },
    check() {
      this.message = this.getColour();
    },
    send() {
      // send data as proper html-form x-www-urlencoded.
      const params = new URLSearchParams();
      params.append('mix', this.mix);
      params.append('effect', this.effect);
      params.append('colorone', this.colorone);
      params.append('colortwo', this.colortwo);
      params.append('rate',this.rate);

      //send data
      axios
        .post('/command', params)
        .then(response => {
          this.message = response.data.content;
          this.hasError = false;
        })
        .catch(error => {
          this.message = `${error}`;
          this.hasError = true;
        });
    },
  },

  // vuelife cycle, what to call after
  // the vue instance is mounted
  mounted() {},
});
