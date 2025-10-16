import tensorflow as tf
import tensorflow.keras.layers as kl
from tensorflow.keras.regularizers import l2
from tensorflow.keras.activations import relu
from keras.saving import register_keras_serializable

try:
  import cupy as np
  np.array([1, 2, 3])
except Exception as e:
  import numpy as np
  # print(f"CuPyが使えないためNumPyを使います: {e}")


# input.shape [ batch_size, 24, 24, 288 ]
# output.shape [ 23 * 23 * 22 ]

@register_keras_serializable()
class AlphaZeroResNet(tf.keras.Model):
  def __init__(self, action_space, n_blocks=3, filters=256, use_bias=False, **kwargs):
    super(AlphaZeroResNet, self).__init__(**kwargs)
    self.action_space = action_space
    self.filters = filters
    self.n_blocks = n_blocks
    self.use_bias = use_bias

    self.conv1 = kl.Conv2D(
        filters,
        kernel_size=3,
        padding="same",
        use_bias=use_bias,
        kernel_regularizer=l2(0.001),
        kernel_initializer="he_normal"
    )

    self.bn1 = kl.BatchNormalization()

    #: residual tower(残差ブロック)
    self.resblocks = [
      ResBlock(filters=self.filters, use_bias=use_bias) for _ in range(self.n_blocks)
    ]

    #: policy head
    self.conv_p = kl.Conv2D(
        2,
        kernel_size=1,
        use_bias=use_bias,
        kernel_regularizer=l2(0.001),
        kernel_initializer="he_normal"
    )
    self.bn_p = kl.BatchNormalization()
    self.flat_p = kl.Flatten()
    self.logits = kl.Dense(
        action_space,
        kernel_regularizer=l2(0.001),
        kernel_initializer="he_normal"
    )

    #: value head
    self.conv_v = kl.Conv2D(
        1,
        kernel_size=1,
        use_bias=use_bias,
        kernel_regularizer=l2(0.001),
        kernel_initializer="he_normal"
    )
    self.bn_v = kl.BatchNormalization()
    self.flat_v = kl.Flatten()
    self.value = kl.Dense(
        1,
        activation="tanh",
        kernel_regularizer=l2(0.001),
        kernel_initializer="he_normal"
    )
    
  def call(self, x, training=False):
    x = relu(self.bn1(self.conv1(x), training=training))

    for resblock in self.resblocks:
        x = resblock(x, training=training)

    #: policy head
    x1 = relu(self.bn_p(self.conv_p(x), training=training))
    x1 = self.flat_p(x1)
    logits = self.logits(x1)
    policy = tf.nn.softmax(logits)

    #: value head
    x2 = relu(self.bn_v(self.conv_v(x), training=training))
    x2 = self.flat_v(x2)
    value = self.value(x2)

    return policy, value

  def predict(self, state):
    if len(state.shape) == 3:
      state = state[np.newaxis, ...]

    policy, value = self(state)

    return policy, value

  def get_config(self):
    config = super().get_config()
    config.update({
      "action_space": self.action_space,
      "n_blocks": self.n_blocks,
      "filters": self.filters,
      "use_bias": self.use_bias
    })
    return config

  @classmethod
  def from_config(cls, config):
    return cls(**config)


@register_keras_serializable()
class ResBlock(tf.keras.layers.Layer):
  def __init__(self, filters, use_bias, **kwargs):
    super(ResBlock, self).__init__(**kwargs)
    self.filters = filters
    self.use_bias = use_bias

    self.conv1 = kl.Conv2D(
        filters, kernel_size=3, padding="same",
        use_bias=use_bias, kernel_regularizer=l2(0.001),
        kernel_initializer="he_normal"
    )
    self.bn1 = kl.BatchNormalization()
    self.conv2 = kl.Conv2D(
        filters, kernel_size=3, padding="same",
        use_bias=use_bias, kernel_regularizer=l2(0.001),
        kernel_initializer="he_normal"
    )
    self.bn2 = kl.BatchNormalization()

  def call(self, x, training=False):
    inputs = x
    x = relu(self.bn1(self.conv1(x), training=training))
    x = self.bn2(self.conv2(x), training=training)
    x = x + inputs
    x = relu(x)
    return x

  def get_config(self):
    config = super().get_config()
    config.update({
      "filters": self.filters,
      "use_bias": self.use_bias
    })
    return config

  @classmethod
  def from_config(cls, config):
    return cls(**config)



