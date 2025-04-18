<script module="@canvas:c1">
    init() {
        print('canas:c1 init start')
    }
    draw(dc,rc) {
        dc.fill('#ccc')
        dc.text('canvas:c1','center')
    }
</script>

<script module="@canvas:c2">
    draw(dc,rc) {
        dc.fill('#ccc')
        dc.text('canvas:c2','center')
    }
</script>

<script module="@canvas:c3">
    draw(dc,rc) {
        dc.fill('#eee')
        dc.text('canvas:c3','center')
    }
</script>