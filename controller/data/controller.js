axios.defaults.headers.common['Content-Type'] =
  'application/x-www-form-urlencoded';

var app = new Vue({
  el: '#app',

  // this is the data relating to the app.
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

  //this filters the data for certain cases, 
  filters: {
    
    // this returns milliseconds to seconds with 2 decimal places, easier to read.
    filterRate: function(value) {
      return `${(value / 1000).toFixed(2)} s`;
    }
  },

  //these are functions or methods you want available in the app.
  methods: {

    //this sends data to the ESP
    send() {

      // construct data as proper html-form x-www-urlencoded, the esp finds this easiest to work with.
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
});
